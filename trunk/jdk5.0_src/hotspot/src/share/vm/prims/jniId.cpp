#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jniId.cpp	1.13 03/07/23 19:00:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

# include "incls/_precompiled.incl"
# include "incls/_jniId.cpp.incl"

// JNI id (jmethodID ) implementation.
// jmethodID and jfieldID are opaque pointers defined by JNI (jni.h).
// In this implementation, a class/index pair maps to and from an id.
// For methods the index is the index into the methods() array of instanceKlass.
// For fields, due to performance requirements in JNI, access
// reverts to the earlier mechanism described in jfieldWorkaround.hpp
// The design goal was to eliminate sequential search for ids, this requires 
// pre-allocated ids.  IDs are only allocated if a jmethodID is 
// used on the class.
// To minimize space usage, one byte is reserved per id.  
// The id is a pointer to that byte.
// Since a byte can't hold sufficiently interesting data, the bytes are
// grouped into buckets and together form a bucket pointer (or 'slot' in the
// bucket table) - each bucket pointer typically points to the default bucket.
// Thus on a 32 bit machine there are four bytes per slot, and thus four ids
// per bucket; and on a 64 bit machine eight bytes per slot and eight ids per
// bucket (ids_per_bucket).
// The bucket points to the map which gives the class and the top of the bucket
// table - which when subtracted from the id gives the index.
// If additional information on a id is needed (for example, method resolution
// caching) a writeable (non-default) bucket is created.
// 
// Mapping:
//
//      address of bucket table + index == id
//
// An example:
//
// Assume a 32 bit machine.  The bucket table must be aligned on a four byte
// boundary, using binary notation, lets say the bucket table is at address
// 101110100, we want the ninth index in the class (or 1001). The id is just
// the sum:
//    101110100
//    +    1001
//    ---------
//    101111101
//
// the slot for this id is computed by masking off the low-order bits:
//    101111101 id
//  & 111111100 id_addr_mask
//    ---------
//    101111100 slot
//
// this id will be the second id in the bucket:
//    101111101 id
//  & 000000011 lower_id_mask
//    ---------
//           01 id within bucket
//
// which only interesting if the bucket is writable.
//
// our example id (index 9) is in the slot mark withed '*'.  The id at index 22 ('$')
// was used in a JNI invoke interface method which needs to cache computed itable
// index, so a writable bucket was created for it.  Thus, for this class only
// two buckets were created (so far), the default read-only (for indices 0-19) and
// one writable bucket (for indices 20-23).  There is no significance that in this
// example it is the last slot that is writable (except that it was easier to draw).
// Since on a 32 bit VM there are four ids per slot, there have to be four detail
// records per bucket.  So the id for index 22 is the third id within the bucket
// and uses the third detail slot.  Note pointers to each structure point to the
// beginning of the structure - where on the structure the arrows point is not
// significant.
//                                                                                     
//                                              +<--------------------------------+    
//                                              |                                 ^    
//     instanceKlass          jniIdMap    +-----+   jniIdBucket (default bucket)  |        
//     ______________       ______________v__   |    _____________________        |      
//    |              |     |    klassOop     |  +<--|-- map               |       |    
//    |              |     |_________________|      |_____________________|       |    
//    | _jni_id_map -|---->| default bucket -|----->|            next   --|---+   |      
//    |              |     |_________________|      |_____________________|   |   |
//    |______________|     | bucket table   -|--+   | empty unused details|   |   |
//                         |_________________|  |   |_____________________|   |   |  
//                                              |   | empty unused details|   |   |
//                                              |   |_____________________|   |   |  
//                                         +----+   | empty unused details|   |   | 
//                                         |        |_____________________|   |   |
//                                         |    +-->| empty unused details|   |   | 
//                           bucket table  |    |   |_____________________|   |   |
//                          _______________v_   |                             |   |
//   indices 0,1,2,3       |     slot      --|->+                             |   |
//                         |_________________|  |                             |   |
//   indices 4,5,6,7       |     slot      --|->+     jniIdBucket         +---+   |    
//                         |_________________|  |     (writable bucket)   |       |
//   indices 8,9*,10,11    |     slot *    --|->+      ___________________v_      |
//                         |_________________|  |     |            map    --|---->+
//   indices 12,13,14,15   |     slot      --|->+     |_____________________|  
//                         |_________________|  |     |       next == NULL  |    
//   indices 16,17,18,19   |     slot      --|->+     |_____________________| 
//                         |_________________|        | empty details       |        
//   indices 20,21,22$,23  |     slot $    --|------> |_____________________| 
//                         |_________________|        | empty details       |        
//                                                    |_____________________| 
//                                                    | itable index        |  $ 
//                                                    |_____________________|            
//                                                    | empty details       |                 
//                                                    |_____________________|            
//                                                                                      

static const intptr_t ids_per_bucket         = sizeof(intptr_t);
static const intptr_t lower_id_mask          = ids_per_bucket - 1;
static const intptr_t id_addr_mask           = ~lower_id_mask;

static const jint ID_MAGIC                   = (jint)(('J' << 24) | ('N' << 16) | ('I' << 8) | 'D');

// forward declarations
class jniIdMap;

//////////////////////////////////////////////////////////////////////
// 
//   jniIdBucket - definition
//
// A jniIdBucket is referenced by every slot in the class' ID table.
// There is one special read-only default bucket.
// A linked list of buckets is kept to optimize GC and deallocation
// traversal of buckets - the default bucket is the head of the list.

class jniIdBucket : public CHeapObj {
 private:
  jniIdMap* _map;                            // the map containing this bucket
  jniIdBucket* _next;                        // the next bucket in this map
  jniIdDetails _details[sizeof(intptr_t)];   // details for each id in bucket.
                                             // use "sizeof(intptr_t)" since "ids_per_bucket"
                                             // won't work at compile time

 public:
  jniIdBucket(jniIdMap* map, jniIdBucket* head);
  inline jniIdMap* map() const                  { return _map; }
  jniIdBucket* default_bucket() const;
  jniIdDetails* details(intptr_t index)         { return &(_details[index]); }
  jniIdBucket* next() const                     { return _next; }

  void set_next(jniIdBucket* next)              { _next = next; }
  jniIdBucket* createWritableBucket();
  int index_for(intptr_t id) const;

  // Garbage collection support
  static void deallocate(jniIdBucket* current);
};


//////////////////////////////////////////////////////////////////////
// 
//   jniIdMap - definition
//
// If any jmethodIDs exist for a class then there is 
// a jniIdMap.  instanceKlass holds a pointer to jniIdMapBase, which 
// always points to a jniIdMap - no instances of the base class are
// ever created.  If the JVMTI RedefineClasses function is used, there
// will be old versions of the class, if a jmethodID is created to 
// reference an obsolete method (if it is still on a execution stack),
// a map will be created in the old class.  A jmethodID created before
// redefinition always tracks the current definition - see the JVMTI 
// spec for details.

class jniIdMap : public jniIdMapBase {
 private:
  klassOop _klass;                 // the class whose ids we are mapping
  int _index_cnt;                  // the number of indicies allowed
  jniIdBucket* _default_bucket;    // read-only bucket used unless writable info needed
  jniIdBucket** _bucket_table;     // the id area, one byte per id - divided in buckets
  void* _bucket_table_mem;         // allocated area for bucket table,
                                   // differs if pointer boundaries require
  jint _magic;                     // if valid, is ID_MAGIC

  jniIdMap(klassOop klass, int index_cnt);
 public:
  ~jniIdMap();
  static jniIdMap* create(instanceKlassHandle ikh);

  inline int index_cnt() const                  { return _index_cnt; }
  inline klassOop klass() const                 { return _klass; }
  inline jniIdBucket** bucket_table() const     { return _bucket_table; }
  inline jniIdBucket* default_bucket() const    { return _default_bucket; }
  inline bool is_valid() const                  { return _magic == ID_MAGIC; }

  static int compute_index_cnt(instanceKlassHandle ikh);
  intptr_t id_for(int index);
  int index_for(intptr_t id) const;

  // Garbage collection support
  void oops_do(OopClosure* f);
  static void deallocate(jniIdMap* map);
};


//////////////////////////////////////////////////////////////////////
// 
//   jniIdBucket - implementation
//

// Construct a bucket.  Pass the map it is part of and the head of the
// bucket linked list.  If head is NULL, we are making the default bucket.
jniIdBucket::jniIdBucket(jniIdMap* map, jniIdBucket* head) {
  _map = map; 
  if (head == NULL) {
    // we are making the head of the list (default bucket)
    _next = NULL;  
  } else {
    // order doesn't matter. insert right after the head
    _next = head->next();
    head->set_next(this);
  }
  for (int i=0; i<sizeof(intptr_t); i++) {
    _details[i].set_itable_index(-1);
  }
}

inline jniIdBucket* jniIdBucket::default_bucket() const { 
  return map()->default_bucket(); 
}

inline int jniIdBucket::index_for(intptr_t id) const { 
  return map()->index_for(id);
}

// something wants to scribble on the bucket details, from the info on
// this read-only default bucket, create a writable copy
jniIdBucket* jniIdBucket::createWritableBucket() {
  return new jniIdBucket(map(), this);
}

// deallocate the linked list of buckets
void jniIdBucket::deallocate(jniIdBucket* current) {
  while (current != NULL) {
    jniIdBucket* next = current->_next;
    current->_next = NULL;
    delete current;
    current = next;
  }
}


//////////////////////////////////////////////////////////////////////
// 
//   jniIdMap - implementation
//

int jniIdMap::compute_index_cnt(instanceKlassHandle ikh) {
  return ikh->methods()->length();
}

jniIdMap::jniIdMap(klassOop klass, int index_cnt) {
  _klass = klass;
  _index_cnt = index_cnt;

  // bucket_count is number of ids divided by the number of ids in a bucket.
  // Round up the division.  An index is zero based.
  int bucket_count = (index_cnt + (ids_per_bucket - 1)) / ids_per_bucket;

  // Add one to allow shifting to an even ids_per_bucket boundary
  _bucket_table_mem = (void*)NEW_C_HEAP_ARRAY(jniIdBucket*, bucket_count + 1);

  // Position bucket_table on an even ids_per_bucket boundary
  intptr_t bucket_table_id = (intptr_t)_bucket_table_mem;
  if (bucket_table_id & lower_id_mask) {
    // the bucket memory isn't at a pointer size even boundary, bump and round
    bucket_table_id = (bucket_table_id + sizeof(jniIdBucket**)) & id_addr_mask;
  }
  _bucket_table = (jniIdBucket**)bucket_table_id;

  assert(((intptr_t)_bucket_table & lower_id_mask) == 0, "low bits must be zero");
  assert(((char*)_bucket_table_mem + (sizeof(jniIdBucket*) * (bucket_count + 1))) >= 
         (char*)(_bucket_table + bucket_count), "repositioned table must fit");
  assert((bucket_count * ids_per_bucket) >= index_cnt, "allocated count cannot be less than count");

  _default_bucket = new jniIdBucket(this, NULL);

  // every bucket slot starts out with the default bucket
  for (int i = 0; i < bucket_count; ++i) {
    _bucket_table[i] = _default_bucket;
  }

  _magic = ID_MAGIC;
}

jniIdMap::~jniIdMap() {
  _magic = 0;
  _klass = NULL;
  FreeHeap(_bucket_table_mem);
  _bucket_table_mem = NULL;

  // delete the bucket linked list
  jniIdBucket::deallocate(_default_bucket);
  _default_bucket = NULL;
}

jniIdMap* jniIdMap::create(instanceKlassHandle ikh) {
  jniIdMap* bab = new jniIdMap(ikh->as_klassOop(), compute_index_cnt(ikh));
  return bab;
}

void jniIdMapBase::deallocate(jniIdMapBase* map_base) {
  delete (jniIdMap*)map_base;
}

// convert an index into a JNI ID
inline intptr_t jniIdMap::id_for(int index) {
  assert(index < index_cnt() && index >= 0, "index must be in range");
  intptr_t top_id = (intptr_t)bucket_table();
  return top_id + index;
}

inline int jniIdMap::index_for(intptr_t id) const {
  return id - (intptr_t)bucket_table();
}

void jniIdMap::oops_do(OopClosure* f) {
  f->do_oop((oop*)&(_klass));
}

void jniIdMapBase::oops_do(OopClosure* f) {
  ((jniIdMap*)this)->oops_do(f);  // jniIdMapBase is always a jniIdMap - convert to that
}

// Verify JNI method identifiers
void jniIdMapBase::verify() {
  // no-op for now
}


//////////////////////////////////////////////////////////////////////
// 
//   jniIdPrivate
//
// Support routines for JNI IDs

class jniIdPrivate : public AllStatic {
 public:
  // return an id for the specified class and index.
  // may grab lock
  static intptr_t id_for(instanceKlassHandle ikh, int index) {
    // Double-checked locking idiom.  Make sure this thread doesn't use
    // stale values for subsequent loads.  This applies not just to the
    // reload of map in the locked region below (which is protected by
    // the act of acquiring the lock), but to any loads reachable from
    // or implied by map.
    jniIdMap* map = (jniIdMap*)ikh->jni_id_map_acquire();
    if (map == NULL) {
      MutexLocker ml(JNIIdentifier_lock);
      // check again with lock held
      map = (jniIdMap*)ikh->jni_id_map_acquire();
      if (map == NULL) {
        // No map, create a map for this class
        map = jniIdMap::create(ikh);
        // Set the new map as the map for this class
        // To prevent an uninitialized instance from being visible in
        // another thread, use a memory barrier to make sure all memory
        // accesses involved in creating the map are completed before
        // the map's address is made visible to other threads.
        ikh->release_set_jni_id_map(map);
        // Don't release the lock until the store of the new map is
        // visible to other threads.  If we didn't do this, the next thread
        // to acquire the lock might see the old value of the map and
        // erroneously create another one.  Releasing the lock has
        // release semantics (i.e., all accesses in the locked region,
        // including the store we just issued, are made visible to other
        // threads before the lock release is made visible), so we don't
        // have to do anything here.
	//OrderAccess::release();
      }
    }
    return map->id_for(index);
  }

  // return an id for the specified class and index.
  // may grab lock
  static inline intptr_t id_for(klassOop k, int index) {
    instanceKlassHandle ikh(k);
    return id_for(ikh, index);
  }

  // return an id if it is already allocated, otherwise return NULL.
  // Must be async-safe.   No allocation should be done and
  // so handles are not used to avoid deadlock.
  static inline intptr_t id_for_or_null(klassOop k, int index) {
    jniIdMap* map = (jniIdMap*)(instanceKlass::cast(k)->jni_id_map());
    if (map == NULL) {
      return NULL;
    }
    return map->id_for(index);
  }

  // from an id, return a pointer to the bucket pointer.
  // simply returns the bucket slot the id is in by masking the 
  // low bits.
  static inline jniIdBucket** bucket_p_for(intptr_t id) {
    intptr_t addr = id & id_addr_mask;
    return (jniIdBucket**)addr;
  }

  static bool id_is_valid(intptr_t id) {
    if (id == 0) {
      return false;
    }
    jniIdBucket** bucket_p =  bucket_p_for(id);
    if (bucket_p == NULL) {
      return false;
    }
    jniIdBucket* bucket =  *bucket_p;
    if (bucket == NULL) {
      return false;
    }
    jniIdMap* map =  bucket->map();
    if (map == NULL || !map->is_valid()) {
      return false;
    }
    int index = map->index_for(id);
    if (index < 0 || index >= map->index_cnt()) {
      return false;
    }
    return true;
  }

  static inline void id_to_klass_and_index(intptr_t id, klassOop* klass_p, int* index_p) {
    jniIdBucket** bucket_p =  bucket_p_for(id);
    jniIdBucket* bucket =  *bucket_p;
    jniIdMap* map =  bucket->map();
    *index_p = map->index_for(id);
    *klass_p = map->klass();
  }

  // Return writable bucket details.
  // May grab lock.
  static jniIdDetails* id_to_klass_index_and_details(intptr_t id, klassOop* klass_p, int* index_p) {
    jniIdBucket** bucket_p       = bucket_p_for(id);
    // Double-checked locking idiom.  Make sure this thread doesn't use
    // stale values for subsequent loads.  This applies not just to the
    // reload of bucket in the locked region below (which is protected by
    // the act of acquiring the lock), but to any loads reachable from or
    // implied by bucket and map.
    jniIdBucket*  bucket         = (jniIdBucket*)OrderAccess::load_ptr_acquire(bucket_p);
    jniIdMap*     map            = bucket->map();
    jniIdBucket*  default_bucket = map->default_bucket();
    if (bucket == default_bucket) {
      MutexLocker ml(JNIIdentifier_lock);
      // Check again with lock held
      bucket = (jniIdBucket*)OrderAccess::load_ptr_acquire(bucket_p);
      if (bucket == default_bucket) {
        // Default bucket isn't writable, create a new one
        bucket = bucket->createWritableBucket();
        // To prevent an uninitialized instance from being visible in
        // another thread, use a memory barrier to make sure all memory
        // accesses involved in creating the bucket are completed
        // before the bucket's address is made visible to other threads.
        OrderAccess::release_store_ptr(bucket_p, bucket);
        // Don't release the lock until the store of the new bucket is
        // visible to other threads.  If we didn't do this, the next thread
        // to acquire the lock might see the old value of the bucket and
        // erroneously create another one.  Releasing the lock has
        // release semantics (i.e., all accesses in the locked region,
        // including the store we just issued, are made visible to other
        // threads before the lock release is made visible), so we don't
        // have to do anything here.
	//OrderAccess::release();
      }
    }
    *index_p = map->index_for(id);
    *klass_p = map->klass();
    intptr_t detail_index = id & lower_id_mask;
    return bucket->details(detail_index);
  }
};


//////////////////////////////////////////////////////////////////////
// 
//   jniIdSupport - implementation
//

// return the jmethodID for a methodOop.  For a redefined method, this 
// will be the most recent equivalent (EMCP, see MethodComparator)
// method, which will either be in the visible class (the visible class
// never changes, old versions are created) or a class which has an
// equivalent method but which is not equivalent to the newer class.
// Note the the method_holder() for all methods is the visible class.
// This implementation assumes that the set of method signatures does
// not change during redefinition and thus the index can be reused.
jmethodID jniIdSupport::to_jmethod_id(methodOop method_oop) {
  int index = method_oop->method_index();
  klassOop k = method_oop->method_holder();
  methodHandle method_handle(method_oop);
  instanceKlassHandle ikh(k);
  if (method_oop->is_old_version()) {
    // this method is obsolete, that is, it has been replaced with RedefineClasses.
    // Look for it in previous versions of the class.
    methodOop curr_method_oop;
    instanceKlassHandle curr_ikh = ikh;
    do {
      if (!curr_ikh->has_previous_version()) {
        assert(false, "obsolete method not found");
        break;
      }
      curr_ikh = curr_ikh->previous_version();

      // method will be at the same index, since we assume no method additions/deletions
      curr_method_oop = methodOop(curr_ikh->methods()->obj_at(index));

      if (curr_method_oop->is_non_emcp_with_new_version()) {
        // this may not yet be the method we are looking for but
        // we want the representitive method (and thus class) to
        // be the oldest non-EMCP method upto and and including
        // the method we are looking for.
        ikh = curr_ikh;
      }
    } while (curr_method_oop != method_handle());
  }
  jmethodID id = (jmethodID)jniIdPrivate::id_for(ikh, index);  // may grab lock - oops above should not be used below
  assert(method_handle()->name() == to_method_oop(id)->name(), "names must match");
  assert(method_handle()->signature() == to_method_oop(id)->signature(), "signatures must match");
  assert(!to_method_oop(id)->is_old_version() || to_method_oop(id)->is_non_emcp_with_new_version(), "returnable version");
  assert(method_handle()->is_old_version() || to_method_oop(id) == method_handle(), "oops should match");
  return id;
}

jmethodID jniIdSupport::to_jmethod_id_or_null(methodOop method_oop) {
  int index = method_oop->method_index();
  klassOop k = method_oop->method_holder();
  return (jmethodID)jniIdPrivate::id_for_or_null(k, index);
}

// obsolete methods cannot be accessed via this function
jmethodID jniIdSupport::to_jmethod_id(klassOop k, int index) {
  assert(instanceKlass::cast(k)->methods()->length() > index, "invalid index");
  jmethodID id = (jmethodID)jniIdPrivate::id_for(k, index);  // may grab lock - oops above should not be used below
  assert(to_method_oop(id)->method_index() == index, "indices should match");
  return id;
}

klassOop jniIdSupport::to_klass_oop(jmethodID id) {
  // cannot use short-cut of directly accessing class because that could return
  // an old version of the class
  return to_method_oop(id)->method_holder();
}

methodOop jniIdSupport::to_method_oop(jmethodID id) {
  assert(jniIdPrivate::id_is_valid((intptr_t)id), "id must be valid");
  klassOop k;
  int index;
  jniIdPrivate::id_to_klass_and_index((intptr_t)id, &k, &index);
  return methodOop(instanceKlass::cast(k)->methods()->obj_at(index));
}

methodOop jniIdSupport::to_method_details(jmethodID id, jniIdDetails** details_p) {
  assert(jniIdPrivate::id_is_valid((intptr_t)id), "id must be valid");
  klassOop k;
  int index;
  *details_p = jniIdPrivate::id_to_klass_index_and_details((intptr_t)id, &k, &index);
  return methodOop(instanceKlass::cast(k)->methods()->obj_at(index));
}

jniIdSupport::CheckResult jniIdSupport::check_method(klassOop test_k, jmethodID id) {
  if (!jniIdPrivate::id_is_valid((intptr_t)id)) {
    return bad_id;
  }
  klassOop method_k;
  int index;
  jniIdPrivate::id_to_klass_and_index((intptr_t)id, &method_k, &index);
  objArrayOop methods = instanceKlass::cast(method_k)->methods();
  if (index >= methods->length()) {
    // there is no such index
    return bad_index;
  }
  if (test_k == NULL) {
    // caller does not want to do this test - we pass
    return valid_id;
  }
  if (!Klass::cast(test_k)->is_subtype_of(method_k)) {
    return not_in_class;
  }
  return valid_id;
}


