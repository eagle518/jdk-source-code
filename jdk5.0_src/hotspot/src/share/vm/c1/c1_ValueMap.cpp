#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_ValueMap.cpp	1.21 03/12/23 16:39:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_ValueMap.cpp.incl"


define_array(intxArray , intx)
define_stack(intxStack , intxArray)



// Implementation of Bucket
//
// A bucket is a list of values and corresponding hash values. New values are added
// at the end; find searches for an equal value. 0 has values indicate that the slot
// has been killed (via a partial kill).

class Bucket: public CompilationResourceObj {
 private:
  Values             _values;                    // the list of values hashed into the same bucket
  intxStack          _hashes;                    // the corresponding list of hash values for fast comparison

#ifndef PRODUCT
  int                _id;                        // the bucket id for printing
  int                _number_of_finds;           // the number of calls to find since the bucket was created
  int                _number_of_hits;            // the number of hits in find since the bucket was created
  int                _total_finds;               // the total number of comparisons since the bucket was created
#endif // PRODUCT

 public:
  // creation
  Bucket::Bucket(int id)
  : _values(ValueMapBucketInitialSize)
  , _hashes(ValueMapBucketInitialSize)
#ifndef PRODUCT
  , _id(id)
  , _number_of_finds(0)
  , _number_of_hits(0)
  , _total_finds(0)
#endif
  {
    assert(ValueMapBucketInitialSize <= ValueMapBucketMaxSize, "initial_size > max_size");
  }

  // accessors
  int max_size() const                           { return ValueMapBucketMaxSize; }
  int size() const                               { return _values.length(); }

#ifndef PRODUCT
  int id() const                                 { return _id; }
  int number_of_finds() const                    { return _number_of_finds; }
  int number_of_hits() const                     { return _number_of_hits; }
  int total_finds() const                        { return _total_finds; }
  float average_find_size() const                { return number_of_finds() > 0 ? (float)total_finds() / (float)number_of_finds() : 0; }
#endif

  // manipulation
  void kill_all();
  void kill_array(ValueType* type);
  void kill_field(ciField* field);
  Value find(Value x, intx hash);
  void append(Value x, intx hash);

  void truncate_to(int new_size) {
    _values.trunc_to(new_size);
    _hashes.trunc_to(new_size);
  }

  int length() const                             { return _values.length(); }
  Value at(int i) const                          { return _values.at(i); }

  void print() PRODUCT_RETURN;
};


void Bucket::kill_all() {
  _values.clear();
  _hashes.clear();
}


void Bucket::kill_array(ValueType* type) {
  for (intx i = size() - 1; i >= 0; i--) {
    Value x = _values.at(i);
    if (x != NULL
    && (x->as_LoadIndexed() != NULL)
    && x->type()->tag() == type->tag()
    ) {
      // memory access => invalidate
      _values.remove_at(i);
      _hashes.remove_at(i);
    }
  }
}


void Bucket::kill_field(ciField* field) {
  for (intx i = size() - 1; i >= 0; i--) {
    Value x = _values.at(i);
    if (x != NULL) {
      LoadField* lf = x->as_LoadField();
      // c1Field's are not unique; must compare their contents
      if (lf != NULL &&
          lf->field()->holder() == field->holder() &&
          lf->field()->offset() == field->offset()) {
        // field access => invalidate
        _values.remove_at(i);
        _hashes.remove_at(i);
      }
    }
  }
}


void Bucket::append(Value x, intx hash) {
  assert(size() < max_size(), "bucket must not exceed maximum size");
  _values.append(x);
  _hashes.append(hash);
}


Value Bucket::find(Value x, intx hash) {
  assert(x != NULL && hash != 0 && x->hash() == hash, "illegal x or hash");
  assert(size() <= max_size(), "bucket must not exceed maximum size");
  NOT_PRODUCT(_number_of_finds++);
  for (intx i = size() - 1; i >= 0; i--) {
    NOT_PRODUCT(_total_finds++);
    if (_hashes.at(i) == hash) {
      Value f = _values.at(i);
      if (f->is_equal(x)) {
        NOT_PRODUCT(_number_of_hits++);
        return f;
      }
    }
  }
  return x;
}


// Implementation of ValueMap
//
// A ValueMap is a list of buckets. Values are hashed into
// buckets via their hash value and an index computation.

ValueMap::ValueMap() :
    _map(ValueMapMaxSize)
#ifndef PRODUCT
  , _number_of_finds(0)
  , _number_of_hits(0)
  , _total_finds(0)
#endif
  , _single_bucket(new Bucket(0)) {
  if (!UseValueNumbering) return;
}

int ValueMap::number_of_buckets() const {
  if (_single_bucket) {
    return 1;
  } else {
    return _map.length();
  }
}


Bucket* ValueMap::bucket_at(int i) {
  if (_single_bucket != NULL) {
    assert(i == 0, "only one");
    return _single_bucket;
  } else {
    return _map.at(i);
  }
}


Bucket* ValueMap::lookup_bucket(intx hash) {
  if (_single_bucket) {
    return _single_bucket;
  } else {
    const intx i = ((hash >> 3) & max_intx)  % _map.length();
    return _map.at(i);
  }
}


void ValueMap::kill_all() {
  if (!UseValueNumbering) return;
  for (int i = 0; i < number_of_buckets(); i++) bucket_at(i)->kill_all();
}


void ValueMap::kill_array(ValueType* type) {
  if (!UseValueNumbering) return;
  for (int i = 0; i < number_of_buckets(); i++) bucket_at(i)->kill_array(type);
}


void ValueMap::kill_field(ciField* field) {
  if (!UseValueNumbering) return;
  for (int i = 0; i < number_of_buckets(); i++) bucket_at(i)->kill_field(field);
}


void ValueMap::resize_bucket(Bucket* bucket) {
  if (bucket == _single_bucket) {
    Bucket* b = _single_bucket;
    _single_bucket = NULL;

    // we've overflowed our single bucket, so let's build a real hashtable
    for (int i = 0; i < ValueMapMaxSize; i++) _map.append(new Bucket(i));

    // insert all the elements from the single bucket into the table
    for (int e = 0; e < bucket->length(); e++) {
      Value x = bucket->at(e);
      const intx h = x->hash();
      assert(h != 0, "can't happen");
      Bucket* bucket = lookup_bucket(h);
      bucket->append(x, h);
    }
    
#ifndef PRODUCT
    _number_of_finds += b->number_of_finds();
    _number_of_hits += b->number_of_hits();
    _total_finds += b->total_finds();
#endif // PRODUCT
  } else {
    // throw away half of the buckets content
    if (bucket->size() >= bucket->max_size()) {
      bucket->truncate_to(bucket->size() / 2);
      if (PrintValueNumbering) {
        tty->print_cr("warning: reduced bucket size");
      }
    }
  }
}


Value ValueMap::find(Value x) {
  // don't do for volatiles or ops that can act as volatiles
  bool is_volatile = false;
  if (x->as_AccessField() != NULL) 
    is_volatile = x->as_AccessField()->field()->is_volatile();
  else if (x->as_UnsafeOp() != NULL)
    is_volatile = true;                  // aliases can act as volatiles
  else if (x->as_Intrinsic() != NULL) {  // CAS acts as as volatile
    ciMethod::IntrinsicId id = x->as_Intrinsic()->id();
    is_volatile =  (id == ciMethod::_compareAndSwapLong_obj ||
                    id == ciMethod::_compareAndSwapInt_obj ||
                    id == ciMethod::_compareAndSwapObject_obj);
  }

  if (UseValueNumbering && !is_volatile) {
    const intx hash = x->hash();
    if (hash != 0) {
      // 0 hash means: exclude from value numbering
      Bucket* bucket = lookup_bucket(hash);
      Value v = bucket->find(x, hash);
      if (v == x) {
        // didn't find anything so add it to the bucket

        // make sure buckets don't become too large, otherwise compilation time
        // becomes too big => for now just reduce bucket if they seem to big -
        // eventually this should become more dynamic and depend on the method
        // and other factors
        if (bucket->size() >= bucket->max_size()) {
          resize_bucket(bucket);
          bucket = lookup_bucket(hash);
        }
        bucket->append(x, hash);
      } else {
#ifndef PRODUCT
        if (PrintValueNumbering) {
          tty->print_cr(
                        "bucket %3d [%2d] (#f = %3d, avg size = %4.1f) %s (%d -> %d)",
                        bucket->id(),
                        bucket->size(),
                        bucket->number_of_finds(),
                        bucket->average_find_size(),
                        x->name(),
                        x->id(),
                        v->id()
          );
        }
#endif // PRODUCT
      }
      return v;
    }
  }
  return x;
}


#ifndef PRODUCT
void ValueMap::print() {
  float hit_rate = 0;
  float avg_size = 0;
  int i, number_of_finds = _number_of_finds, number_of_hits = _number_of_hits, total_finds = _total_finds;
  for (i = 0; i < number_of_buckets(); i++) {
    Bucket* bucket = bucket_at(i);
    number_of_finds += bucket->number_of_finds();
    number_of_hits += bucket->number_of_hits();
    total_finds += bucket->total_finds();
  }

  if (number_of_hits > 0) {
    hit_rate = (float)total_finds / number_of_hits;
  }
  if (number_of_finds > 0) {
    avg_size = (float)total_finds / number_of_finds;
  }
  tty->print_cr("ValueMap %d buckets (#f = %3d, avg size = %5.3f, hit rate=%5.3f)", number_of_buckets(), number_of_finds, avg_size, hit_rate);
  for (i = 0; i < number_of_buckets(); i++) {
    bucket_at(i)->print();
  }
}


void Bucket::print() {
  tty->print_cr("  bucket %3d [%2d] (#f = %3d, avg size = %5.3f)", id(), size(), number_of_finds(), average_find_size());
}

#endif

