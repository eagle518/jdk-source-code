#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_ValueMap.hpp	1.12 03/12/23 16:39:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Bucket;
define_array(BucketArray, Bucket*)
define_stack(BucketList, BucketArray)


class ValueMap: public CompilationResourceObj {
 private:
  BucketList _map;
  Bucket*    _single_bucket;

#ifndef PRODUCT
  int                _number_of_finds;           // the number of calls to find since the valuemap was created
  int                _number_of_hits;            // the number of hits in find since the valuemap was created
  int                _total_finds;               // the total number of comparisons since the valuemap was created
#endif // PRODUCT

  // accessors
  int number_of_buckets() const;
  Bucket* bucket_at(int index);
  Bucket* lookup_bucket(intx hash);
  void resize_bucket(Bucket* bucket);

#ifndef PRODUCT
  int number_of_finds() const                    { return _number_of_finds; }
  int number_of_hits() const                     { return _number_of_hits; }
  int total_finds() const                        { return _total_finds; }
  float average_find_size() const                { return number_of_finds() > 0 ? (float)total_finds() / (float)number_of_finds() : 0; }
#endif // PRODUCT

 public:
  // creation
  ValueMap();

  // manipulation
  void kill_all();
  void kill_array(ValueType* type);
  void kill_field(ciField* field);
  Value find(Value x);

  // debugging/printing
  void print() PRODUCT_RETURN;
};

