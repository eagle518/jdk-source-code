#ifdef USE_PRAGMA_IDENT_SCR
#pragma ident "@(#)jvmtiTagMap.cpp	1.33 04/02/10 16:17:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmtiTagMap.cpp.incl"

// JvmtiTagHashmapEntry 
//
// Each entry encapsulates a JNI weak reference to the tagged object
// and the tag value. In addition an entry includes a next pointer which
// is used to chain entries together.

class JvmtiTagHashmapEntry : public CHeapObj {
 private:
  friend class JvmtiTagMap;

  jweak _object;			// JNI weak ref to tagged object  
  jlong _tag;				// the tag
  JvmtiTagHashmapEntry* _next;		// next on the list

  inline void init(jweak object, jlong tag) {
    _object = object;    
    _tag = tag;
    _next = NULL;
  }

  // constructor
  JvmtiTagHashmapEntry(jweak object, jlong tag)		{ init(object, tag); }

 public:  

  // accessor methods
  inline jweak object() const				{ return _object; }
  inline jlong tag() const				{ return _tag; }

  inline void set_tag(jlong tag) { 
    assert(tag != 0, "can't be zero");
    _tag = tag;
  }

  inline JvmtiTagHashmapEntry* next() const		{ return _next; }
  inline void set_next(JvmtiTagHashmapEntry* next)	{ _next = next; }
};


// JvmtiTagHashmap
// 
// A hashmap is essentially a table of pointers to entries. Entries
// are hashed to a location, or position in the table, and then 
// chained from that location. The "key" for hashing is address of
// the object, or oop. The "value" is the JNI weak reference to the
// object and the tag value. Keys are not stored with the entry. 
// Instead the weak reference is resolved to obtain the key.
//
// A hashmap maintains a count of the number entries in the hashmap
// and resizes if the number of entries exceeds a given threshold.
// The threshold is specified as a percentage of the size - for 
// example a threshold of 0.75 will trigger the hashmap to resize
// if the number of entries is >75% of table size. 
//
// A hashmap provides functions for adding, removing, and finding
// entries. It also provides a function to iterate over all entries
// in the hashmap.

class JvmtiTagHashmap : public CHeapObj {
 private:
  friend class JvmtiTagMap;

  static int _sizes[];			// array of possible hashmap sizes
  int _size;				// actual size of the table
  int _size_index;			// index into size table

  int _entry_count;			// number of entries in the hashmap

  float _load_factor;			// load factor as a % of the size
  int _threshold;			// computed threshold to trigger resizing.

  JvmtiTagHashmapEntry** _table;	// the table of entries.

  // initialize the hashmap
  void init(int size_index=0, float load_factor=0.75f) {
    int initial_size =  _sizes[size_index];
    _size_index = size_index;
    _size = initial_size;
    _entry_count = 0;
    _load_factor = load_factor;
    _threshold = _load_factor * _size;
    _table = NEW_C_HEAP_ARRAY(JvmtiTagHashmapEntry*, initial_size);    
    for (int i=0; i<initial_size; i++) {
      _table[i] = NULL;
    }
  }

  // hash a given key (oop) with the specified size
  static unsigned int hash(oop key, int size) {
    // shift right to get better distribution (as these bits will be zero
    // with aligned addresses)
    unsigned int addr = (unsigned int)((intptr_t)key);
#ifdef _LP64
    return (addr >> 3) % size;
#else
    return (addr >> 2) % size;
#endif
  }

  // hash a given key (oop)
  unsigned int hash(oop key) {
    return hash(key, _size);
  }

  // resize the hashmap - allocates a large table and re-hashes
  // all entries into the new table.
  void resize() {
    int new_size_index = _size_index+1;
    int new_size = _sizes[new_size_index];
    if (new_size < 0) {
      // hashmap already at maximum capacity
      return;
    }

    // allocate new table
    JvmtiTagHashmapEntry** new_table = NEW_C_HEAP_ARRAY(JvmtiTagHashmapEntry*, new_size);
    if (new_table == NULL) {
      return;
    }

    // initialize new table
    int i;
    for (i=0; i<new_size; i++) {
      new_table[i] = NULL;
    }

    // rehash all entries into the new table
    for (i=0; i<_size; i++) {
      JvmtiTagHashmapEntry* entry = _table[i];
      while (entry != NULL) {
	JvmtiTagHashmapEntry* next = entry->next();
	oop key = JNIHandles::resolve(entry->object());	
	assert(key != NULL, "jni weak reference cleared!!");
	unsigned int h = hash(key, new_size);
	JvmtiTagHashmapEntry* anchor = new_table[h];
	if (anchor == NULL) {
	  new_table[h] = entry;
	  entry->set_next(NULL);
	} else {
	  entry->set_next(anchor);
	  new_table[h] = entry;
	}	
	entry = next;
      }
    }

    // free old table and update settings.
    FREE_C_HEAP_ARRAY(JvmtiTagHashmapEntry*, _table);
    _table = new_table;
    _size_index = new_size_index;
    _size = new_size;

    // compute new resize threshold
    _threshold = _load_factor * _size;
  }


  // internal remove function - remove an entry at a given position in the
  // table. 
  inline void remove(JvmtiTagHashmapEntry* prev, int pos, JvmtiTagHashmapEntry* entry) {
    assert(pos >= 0 && pos < _size, "out of range");
    if (prev == NULL) {
      _table[pos] = entry->next();
    } else {
      prev->set_next(entry->next());
    }
    assert(_entry_count > 0, "checking");
    _entry_count--;
  }

 public:

  // create a JvmtiTagHashmap of a preferred size and optionally a load factor.
  // The preferred size is rounded down to an actual size.
  JvmtiTagHashmap(int size, float load_factor=0.0f) {
    int i=0;
    while (_sizes[i] < size) {
      if (_sizes[i] < 0) {
	assert(i > 0, "sanity check");
	i--;
	break;
      }
      i++;
    }

    // if a load factor is specified then use it, otherwise use default
    if (load_factor > 0.01f) {	  
      init(i, load_factor);
    } else {
      init(i);
    }
  }

  // create a JvmtiTagHashmap with default settings
  JvmtiTagHashmap() {
    init();
  }


  // accessors
  int size() const				{ return _size; }   
  JvmtiTagHashmapEntry** table() const		{ return _table; }


  // find an entry in the hashmap, returns NULL if not found.
  inline JvmtiTagHashmapEntry* find(oop key) {
    unsigned int h = hash(key);
    JvmtiTagHashmapEntry* entry = _table[h];
    while (entry != NULL) {
      oop orig_key = JNIHandles::resolve(entry->object());	
      assert(orig_key != NULL, "jni weak reference cleared!!");
      if (key == orig_key) {
	break;
      } 
      entry = entry->next();
    }
    return entry;
  }


  // add a new entry to hashmap 
  inline void add(oop key, JvmtiTagHashmapEntry* entry) {
    assert(key != NULL, "checking");
    assert(find(key) == NULL, "duplicate detected");
    unsigned int h = hash(key);
    JvmtiTagHashmapEntry* anchor = _table[h];
    if (anchor == NULL) {
      _table[h] = entry;
      entry->set_next(NULL);
    } else {
      entry->set_next(anchor);
      _table[h] = entry;
    }

    _entry_count++;

    // if the number of entries exceed the threshold then resize
    if (_entry_count > _threshold) {
      resize();
    }
  }

  // remove an entry with the given key.
  inline JvmtiTagHashmapEntry* remove(oop key) {
    unsigned int h = hash(key);
    JvmtiTagHashmapEntry* entry = _table[h];
    JvmtiTagHashmapEntry* prev = NULL; 
    while (entry != NULL) {
      oop orig_key = JNIHandles::resolve(entry->object());	
      assert(orig_key != NULL, "jni weak reference cleared!!");
      if (key == orig_key) {
	break;
      } 
      prev = entry;
      entry = entry->next();
    }
    if (entry != NULL) {
      remove(prev, h, entry);
    }
    return entry;
  }

  // iterate over all entries in the hashmap 
  void entry_iterate(JvmtiTagHashmapEntryClosure* closure);
};

// possible hashmap sizes - odd primes that roughly double in size.
// To avoid excessive resizing the odd primes from 4801-76831 and
// 76831-307261 have been removed. The list must be terminated by -1.
int JvmtiTagHashmap::_sizes[] =  { 4801, 76831, 307261, 614563, 1228891, 
    2457733, 4915219, 9830479, 19660831, 39321619, 78643219, -1 };


// A supporting class for iterating over all entries in Hashmap
class JvmtiTagHashmapEntryClosure {
 public:
  virtual void do_entry(JvmtiTagHashmapEntry* entry) = 0;
};


// iterate over all entries in the hashmap 
void JvmtiTagHashmap::entry_iterate(JvmtiTagHashmapEntryClosure* closure) {
  for (int i=0; i<_size; i++) {
    JvmtiTagHashmapEntry* entry = _table[i];
    JvmtiTagHashmapEntry* prev = NULL;
    while (entry != NULL) {
      // obtain the next entry before invoking do_entry - this is
      // necessary because do_entry may remove the entry from the
      // hashmap.
      JvmtiTagHashmapEntry* next = entry->next();
      closure->do_entry(entry);		
      entry = next;
     }
  }
}


// memory region for young generation
MemRegion JvmtiTagMap::_young_gen;

// get the memory region used for the young generation
void JvmtiTagMap::get_young_generation() {
  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    _young_gen = gch->get_gen(0)->reserved();
  } else {
    ParallelScavengeHeap* psh = ParallelScavengeHeap::heap();
    _young_gen= psh->young_gen()->reserved();
  }
}

// returns true if oop is in the young generation
inline bool JvmtiTagMap::is_in_young(oop o) {
  assert(_young_gen.start() != NULL, "checking");
  void* p = (void*)o;
  bool in_young = _young_gen.contains(p);
  return in_young;
}

// returns the appropriate hashmap for a given object
inline JvmtiTagHashmap* JvmtiTagMap::hashmap_for(oop o) {
  if (is_in_young(o)) {
    return _hashmap[0];
  } else {
    return _hashmap[1];
  }
}


// create a JvmtiTagMap
JvmtiTagMap::JvmtiTagMap(JvmtiEnv* env) :
  _env(env),
  _lock(Mutex::nonleaf+2, "JvmtiTagMap._lock", false),
  _free_entries(NULL)
{
  assert(JvmtiThreadState_lock->is_locked(), "sanity check");
  assert(((JvmtiEnvBase *)env)->tag_map() == NULL, "tag map already exists for environment");

  // create the hashmaps
  for (int i=0; i<_n_hashmaps; i++) {
    _hashmap[i] = new JvmtiTagHashmap();
  }

  // get the memory region used by the young generation
  get_young_generation(); 

  // finally add us to the environment
  ((JvmtiEnvBase *)env)->set_tag_map(this);
}


// destroy a JvmtiTagMap
JvmtiTagMap::~JvmtiTagMap() {

  // no lock acquired as we assume the enclosing environment is 
  // also being destroryed.
  ((JvmtiEnvBase *)_env)->set_tag_map(NULL);

  // iterate over the hashmaps and destroy each of the entries
  for (int i=0; i<_n_hashmaps; i++) {
    JvmtiTagHashmap* hashmap = _hashmap[i];
    JvmtiTagHashmapEntry** table = hashmap->table();
    for (int j=0; j<hashmap->size(); j++) {
      JvmtiTagHashmapEntry *entry = table[j];
      while (entry != NULL) {
	JvmtiTagHashmapEntry* next = entry->next();
	jweak ref = entry->object();
        JNIHandles::destroy_weak_global(ref);
	delete entry;
	entry = next;
      }
    }

    // finally destroy the hashmap
    delete hashmap;
  }

  // remove any entries on the free list
  JvmtiTagHashmapEntry* entry = _free_entries;
  while (entry != NULL) {
    JvmtiTagHashmapEntry* next = entry->next();
    delete entry;
    entry = next;
  }
}

// create a hashmap entry
// - if there's an entry on the (per-environment) free list then this
// is returned. Otherwise an new entry is allocated.
JvmtiTagHashmapEntry* JvmtiTagMap::create_entry(jweak ref, jlong tag) {
  assert(Thread::current()->is_VM_thread() || is_locked(), "checking");
  JvmtiTagHashmapEntry* entry;
  if (_free_entries == NULL) {
    entry = new JvmtiTagHashmapEntry(ref, tag);
  } else {
    entry = _free_entries;
    _free_entries = entry->next();
    entry->init(ref, tag);
  }
  return entry;
}

// destroy an entry by returning it to the free list
void JvmtiTagMap::destroy_entry(JvmtiTagHashmapEntry* entry) {
  assert(Thread::current()->is_VM_thread() || is_locked(), "checking");
  entry->set_next(_free_entries);
  _free_entries = entry;
}

// returns the tag map for the given environments. If the tag map
// doesn't exist then it is created.
JvmtiTagMap* JvmtiTagMap::tag_map_for(JvmtiEnv* env) {
  JvmtiTagMap* tag_map = ((JvmtiEnvBase *)env)->tag_map();
  if (tag_map == NULL) {
    MutexLocker mu(JvmtiThreadState_lock);
    tag_map = ((JvmtiEnvBase *)env)->tag_map();
    if (tag_map == NULL) {
      tag_map = new JvmtiTagMap(env);
    }
  }
  return tag_map;
}

// iterate over all entries in the tag map.
void JvmtiTagMap::entry_iterate(JvmtiTagHashmapEntryClosure* closure) {
  for (int i=0; i<_n_hashmaps; i++) {      
    JvmtiTagHashmap* hashmap = _hashmap[i];
    hashmap->entry_iterate(closure);
  }
}


// Return the tag value for an object, or 0 if the object is
// not tagged
//
static inline jlong tag_for(JvmtiTagMap* tag_map, oop o) {
  JvmtiTagHashmapEntry* entry = tag_map->hashmap_for(o)->find(o);
  if (entry == NULL) {
    return 0;
  } else {
    return entry->tag();
  }  
}

// If the object is a java.lang.Class then return the klassOop, 
// otherwise return the original object
static inline oop klassOop_if_java_lang_Class(oop o) {
  if (o->klass() == SystemDictionary::class_klass()) {
    if (!java_lang_Class::is_primitive(o)) {
      o = (oop)java_lang_Class::as_klassOop(o);
      assert(o != NULL, "class for non-primitive mirror must exist");      
    }
  }
  return o;
}

// A CallbackWrapper is a support class for querying and tagging an object
// around a callback to a profiler. The constructor does pre-callback
// work to get the tag value, klass tag value, ... and the destructor 
// does the post-callback work of tagging or untagging the object.
//
// {
//   CallbackWrapper wrapper(o); 
//
//   (*callback)(wrapper.klass_tag(), wrapper.obj_size(), wrapper.obj_tag_p(), ...)
//
// } // wrapper goes out of scope here which results in the destructor 
//      checking to see if the object has been tagged, untagged, or the
//	tag value has changed.
//	
class CallbackWrapper : public StackObj {
 private:
  JvmtiTagMap* _tag_map;
  JvmtiTagHashmap* _hashmap;
  JvmtiTagHashmapEntry* _entry;
  oop _o;
  jlong _obj_size;
  jlong _obj_tag;
  jlong _klass_tag;

 public:
  CallbackWrapper(JvmtiTagMap* tag_map, oop o) {  
    assert(Thread::current()->is_VM_thread() || tag_map->is_locked(), 
	   "MT unsafe or must be VM thread");
   
    // for Classes the klassOop is tagged
    _o = klassOop_if_java_lang_Class(o);  

    // object size
    _obj_size = _o->size() * wordSize;

    // record the context
    _tag_map = tag_map;
    _hashmap = tag_map->hashmap_for(_o);
    _entry = _hashmap->find(_o);

    // get object tag 
    _obj_tag = (_entry == NULL) ? 0 : _entry->tag();

    // get the class's tag value
    if (_o == o) {
      _klass_tag = tag_for(tag_map, _o->klass());  
    } else {
      // if the object represents a runtime class then use the
      // tag for java.lang.Class
      _klass_tag = tag_for(tag_map, SystemDictionary::class_klass());
    }
  }

  ~CallbackWrapper() {
    if (_entry == NULL) {      
      if (_obj_tag != 0) {	
	// callback has tagged the object
	assert(Thread::current()->is_VM_thread(), "must be VMThread");
	HandleMark hm;
	Handle h(_o);      
	jweak ref = JNIHandles::make_weak_global(h);	
	JvmtiTagHashmapEntry* entry = _tag_map->create_entry(ref, _obj_tag);
	_hashmap->add(_o, entry);
      }
    } else {
      // object was previously tagged - the callback may have untagged
      // the object or changed the tag value
      if (_obj_tag == 0) {		
	jweak ref = _entry->object();
	
	JvmtiTagHashmapEntry* entry_removed = _hashmap->remove(_o);
	assert(entry_removed == _entry, "checking");   
	_tag_map->destroy_entry(_entry);
	
	JNIHandles::destroy_weak_global(ref);	
      } else {
	if (_obj_tag != _entry->tag()) {
	  _entry->set_tag(_obj_tag);
	}	
      }     
    }
  }

  inline jlong* obj_tag_p()			{ return &_obj_tag; } 
  inline jlong obj_size() const			{ return _obj_size; }
  inline bool obj_tagged() const		{ return _entry != NULL; }
  inline jlong klass_tag() const		{ return _klass_tag; } 
};

// tag an object
//
// This function is performance critical. If many threads attempt to tag objects 
// around the same time then it's possible that the Mutex associated with the 
// tag map will be a hot lock. Eliminating this lock will not eliminate the issue
// because creating a JNI weak reference requires acquiring a global lock also.
void JvmtiTagMap::set_tag(jobject object, jlong tag) {
  MutexLocker ml(lock());

  // resolve the object 
  oop o = JNIHandles::resolve_non_null(object);

  // for Classes we tag the klassOop
  o = klassOop_if_java_lang_Class(o);

  // see if the object is already tagged
  JvmtiTagHashmap* hashmap = hashmap_for(o);
  JvmtiTagHashmapEntry* entry = hashmap->find(o);

  // if the object is not already tagged then we tag it
  if (entry == NULL) {
    if (tag != 0) {
      HandleMark hm;
      Handle h(o);      
      jweak ref = JNIHandles::make_weak_global(h);	

      // the object may have moved because make_weak_global may
      // have blocked - thus it is necessary resolve the handle 
      // and re-hash the object.
      o = h();
      entry = create_entry(ref, tag);
      hashmap_for(o)->add(o, entry);  
    } else {
      // no-op
    }
  } else {
    // if the object is already tagged then we either update
    // the tag (if a new tag value has been provided)
    // or remove the object if the new tag value is 0. 
    // Removing the object requires that we also delete the JNI
    // weak ref to the object.
    if (tag == 0) {
      jweak ref = entry->object();
      hashmap->remove(o);
      destroy_entry(entry);
      JNIHandles::destroy_weak_global(ref);           
    } else {
      entry->set_tag(tag);
    }
  }
}

// get the tag for an object
jlong JvmtiTagMap::get_tag(jobject object) {
  MutexLocker ml(lock()); 

  // resolve the object 
  oop o = JNIHandles::resolve_non_null(object);

  // for Classes get the tag from the klassOop
  return tag_for(this, klassOop_if_java_lang_Class(o));
}


// invoke the heap object callback 
static inline bool doHeapObjectCallback(JvmtiTagMap* tag_map, oop o, jvmtiHeapObjectFilter object_filter, 
					jvmtiHeapObjectCallback heap_object_callback,
                                        void* user_data)
{  
  CallbackWrapper wrapper(tag_map, o);

  // if the object is tagged and we're only interested in untagged objects
  // then don't invoke the callback. Similiarly, if the object is untagged
  // and we're only interested in tagged objects we skip the callback.
  if (wrapper.obj_tagged()) {
    if (object_filter == JVMTI_HEAP_OBJECT_UNTAGGED) {
      return JNI_TRUE;
    }  
  } else {
    if (object_filter == JVMTI_HEAP_OBJECT_TAGGED) {
      return JNI_TRUE;
    }
  }

  jvmtiIterationControl control = (*heap_object_callback)(wrapper.klass_tag(), wrapper.obj_size(), 
                                                          wrapper.obj_tag_p(), 
                                                          user_data);
  return control != JVMTI_ITERATION_ABORT;
}

// support class for IterateOverHeap and IterateOverInstanceOfClass

class IterateOverHeapObjectClosure: public ObjectClosure {
 private:
  JvmtiTagMap* _tag_map;
  klassOop _k_oop;
  jvmtiHeapObjectFilter _object_filter;
  jvmtiHeapObjectCallback _heap_object_callback;
  void* _user_data;
  jboolean _continue;
 public:
  IterateOverHeapObjectClosure(JvmtiTagMap* tag_map, klassOop k_oop, jvmtiHeapObjectFilter object_filter, 
			       jvmtiHeapObjectCallback heap_object_callback,
                               void* user_data)     
  {
    _tag_map = tag_map;
    _k_oop = k_oop;
    _object_filter = object_filter;
    _heap_object_callback = heap_object_callback;
    _user_data = user_data;
    _continue = JNI_TRUE;
  }

  void do_object(oop o) {   
    // check if iteration has been halted
    if (!_continue) {
      return;
    }   

    // ignore any objects that aren't visible to profiler
    if (!ServiceUtil::visible_oop(o)) {
      return;
    } 
    
    // instanceof check when filtering by klass
    if (_k_oop != NULL) {
      if (!o->is_a(_k_oop)) {
          return;      
      }
    }

    // invoke the callback
    _continue = doHeapObjectCallback(_tag_map, o, _object_filter, _heap_object_callback, _user_data);
  }
};


class IterateOverHeapOperation: public VM_Operation {
 private:
  JvmtiTagMap* _tag_map;
  jvmtiHeapObjectFilter _object_filter;
  jvmtiHeapObjectCallback _heap_object_callback;
  void* _user_data;
  klassOop _k_oop;
 public:
   IterateOverHeapOperation(JvmtiTagMap* tag_map, jvmtiHeapObjectFilter object_filter, jvmtiHeapObjectCallback heap_object_callback, void* user_data) :
     _tag_map(tag_map),
     _k_oop(NULL),
     _object_filter(object_filter),
     _heap_object_callback(heap_object_callback),
     _user_data(user_data) {
   }
   IterateOverHeapOperation(JvmtiTagMap* tag_map, klassOop k_oop, jvmtiHeapObjectFilter object_filter, jvmtiHeapObjectCallback heap_object_callback, void* user_data):
     _tag_map(tag_map),
     _k_oop(k_oop),
     _object_filter(object_filter),
     _heap_object_callback(heap_object_callback),
     _user_data(user_data) {
   }
  void doit() {             
    HandleMark hm;    

    IterateOverHeapObjectClosure closure(_tag_map, _k_oop, _object_filter, _heap_object_callback, _user_data);
    Universe::heap()->ensure_parseability();

    // Verify heap before iteration - if the heap gets corrupted then 
    // JVMTI's IterateOverHeap will crash.
    if (VerifyBeforeIteration) {
      Universe::verify();
    }

    Universe::heap()->object_iterate(&closure);
  }

  // GC support - we're holding an oop that might move
  void oops_do(OopClosure *f) {
    if (_k_oop != NULL) {
      f->do_oop((oop*)&_k_oop);	 
    }
  }
  const char* name() const { return "JVMTI IterateOverHeap object closure"; }
};


// iterate over heap functions

void JvmtiTagMap::iterate_over_heap(jvmtiHeapObjectFilter object_filter,
                                    jvmtiHeapObjectCallback heap_object_callback, 
                                    void* user_data) {
  MutexLocker ml(Heap_lock);
  IterateOverHeapOperation op(this, object_filter, heap_object_callback, user_data);
  VMThread::execute(&op);     
}

void JvmtiTagMap::iterate_over_instances_of_class(klassOop k_oop, jvmtiHeapObjectFilter object_filter, 
						  jvmtiHeapObjectCallback heap_object_callback, 
                                                  void* user_data) 
{
  MutexLocker ml(Heap_lock);
  IterateOverHeapOperation op(this, k_oop, object_filter, heap_object_callback, user_data);
  VMThread::execute(&op);  
}

// support class for get_objects_with_tags

class TagObjectCollector : public JvmtiTagHashmapEntryClosure {
 private:
  JvmtiEnv* _env;
  jlong* _tags;
  jint _tag_count;

  GrowableArray<jobject>* _object_results;  // collected objects (JNI weak refs)
  GrowableArray<uint64_t>* _tag_results;    // collected tags

 public:
  TagObjectCollector(JvmtiEnv* env, const jlong* tags, jint tag_count) {
    _env = env;
    _tags = (jlong*)tags;
    _tag_count = tag_count;
    _object_results = new (ResourceObj::C_HEAP) GrowableArray<jobject>(1,true);
    _tag_results = new (ResourceObj::C_HEAP) GrowableArray<uint64_t>(1,true);
  }

  ~TagObjectCollector() {
    _object_results->clear_and_deallocate();
    FreeHeap(_object_results);
    _tag_results->clear_and_deallocate();
    FreeHeap(_tag_results);   
  }

  // for each tagged object check if the tag value matches
  // - if it matches then we create a JNI local reference to the object
  // and record the reference and tag value.
  //
  void do_entry(JvmtiTagHashmapEntry* entry) {
    for (int i=0; i<_tag_count; i++) {      
      if (_tags[i] == entry->tag()) {
	oop o = JNIHandles::resolve(entry->object());
	assert(o != NULL && o != JNIHandles::deleted_handle(), "sanity check");

	// the mirror is tagged
	if (o->is_klass()) {
	  klassOop k = (klassOop)o;
	  o = Klass::cast(k)->java_mirror();
	}

	jobject ref = JNIHandles::make_local(JavaThread::current(), o);
	_object_results->append(ref);
	_tag_results->append((uint64_t)entry->tag());
      }
    }
  }

  // return the results from the collection
  //
  jvmtiError result(jint* count_ptr, jobject** object_result_ptr, jlong** tag_result_ptr) {
    jvmtiError error;
    int count = _object_results->length();
    assert(count >= 0, "sanity check");    

    // if object_result_ptr is not NULL then allocate the result and copy
    // in the object references.
    if (object_result_ptr != NULL) {
      error = _env->Allocate(count * sizeof(jobject), (unsigned char**)object_result_ptr);
      if (error != JVMTI_ERROR_NONE) {
        return error;
      }
      for (int i=0; i<count; i++) {
        (*object_result_ptr)[i] = _object_results->at(i);
      }
    }

    // if tag_result_ptr is not NULL then allocate the result and copy
    // in the tag values.
    if (tag_result_ptr != NULL) {
      error = _env->Allocate(count * sizeof(jlong), (unsigned char**)tag_result_ptr);
      if (error != JVMTI_ERROR_NONE) {	
	if (object_result_ptr != NULL) {
          _env->Deallocate((unsigned char*)object_result_ptr);
	}
        return error;
      }
      for (int i=0; i<count; i++) {
	(*tag_result_ptr)[i] = (jlong)_tag_results->at(i);
      }
    }

    *count_ptr = count;
    return JVMTI_ERROR_NONE;
  }
};

// return the list of objects with the specified tags
jvmtiError JvmtiTagMap::get_objects_with_tags(const jlong* tags, 
  jint count, jint* count_ptr, jobject** object_result_ptr, jlong** tag_result_ptr) {

  TagObjectCollector collector(env(), tags, count); 
  {    
    // iterate over all tagged objects
    MutexLocker ml(lock());
    entry_iterate(&collector);    
  }
  return collector.result(count_ptr, object_result_ptr, tag_result_ptr);
}


// ObjectMarker is used to support the marking objects when walking the 
// heap.
//
// This implementation uses the existing mark bits in an object for
// marking. Objects that are marked must later have their headers restored.
// As most objects are unlocked and don't have their identity hash computed
// we don't have to save their headers. Instead we save the headers that
// are "interesting". Later when the headers are restored this implementation
// restores all headers to their initial value and then restores the few
// objects that had interesting headers.
//
// Future work: This implementation currently uses growable arrays to save
// the oop and header of interesting objects. As an optimization we could
// use the same technique as the GC and make use of the unused area 
// between top() and end().
//

// An ObjectClosure used to restore the mark bits of an object
class RestoreMarksClosure : public ObjectClosure {
 public:  
  void do_object(oop o) {   
    if (o != NULL) {
      markOop mark = o->mark();
      if (mark->is_marked()) {
        o->set_mark(mark->clear_lock_bits()->set_unlocked());
      }
    }    
  }
};

// ObjectMarker provides the mark and visited functions
class ObjectMarker : AllStatic {
 private:
  // saved headers
  static GrowableArray<oop>* _saved_oop_stack;	    
  static GrowableArray<markOop>* _saved_mark_stack;

 public:
  static void init();			    // initialize
  static void done();			    // clean-up

  static inline void mark(oop o);	    // mark an object
  static inline bool visited(oop o);	    // check if object has been visited
};

GrowableArray<oop>* ObjectMarker::_saved_oop_stack = NULL;
GrowableArray<markOop>* ObjectMarker::_saved_mark_stack = NULL;

// initialize ObjectMarker - prepares for object marking
void ObjectMarker::init() {
  assert(Thread::current()->is_VM_thread(), "must be VMThread");

  // prepare heap for iteration 
  Universe::heap()->ensure_parseability();

  // create stacks for interesting headers
  _saved_mark_stack = new (ResourceObj::C_HEAP) GrowableArray<markOop>(4000, true);
  _saved_oop_stack = new (ResourceObj::C_HEAP) GrowableArray<oop>(4000, true);
}

// Object marking is done so restore object headers
void ObjectMarker::done() {
  // iterate over all objects and restore the mark bits to
  // their initial value
  RestoreMarksClosure blk;
  Universe::heap()->object_iterate(&blk);  

  // now restore the interesting headers
  for (int i = 0; i < _saved_oop_stack->length(); i++) {   
    oop o = _saved_oop_stack->at(i);
    markOop mark = _saved_mark_stack->at(i);
    o->set_mark(mark);      
  }

  // free the stacks
  _saved_oop_stack->clear_and_deallocate();
  FreeHeap(_saved_oop_stack);
  _saved_mark_stack->clear_and_deallocate();
  FreeHeap(_saved_mark_stack);  
}

// mark an object 
inline void ObjectMarker::mark(oop o) {
  assert(Universe::heap()->is_in(o), "sanity check");
  assert(!o->mark()->is_marked(), "should only mark an object once");

  // object's mark word
  markOop mark = o->mark();

  // the only marks we don't have to save are unlocked ones with no hash.  
  if (mark != markOopDesc::prototype()) {
    if (mark->must_be_preserved()) {                
      _saved_mark_stack->push(mark);
      _saved_oop_stack->push(o);      
    }
  }

  // mark the object
  o->set_mark(markOopDesc::prototype()->set_marked());
}

// return true if object is marked
inline bool ObjectMarker::visited(oop o) {
  return o->mark()->is_marked();  
}

// Stack allocated class to help ensure that ObjectMarker is used
// correctly. Constructor initializes ObjectMarker, destructor calls
// ObjectMarker's done() function to restore object headers.
class ObjectMarkerController : public StackObj {
 public:
  ObjectMarkerController() {
    ObjectMarker::init();
  }
  ~ObjectMarkerController() {
    ObjectMarker::done();
  }

};


// Utility class to report roots and object references to the profiler.
//
// This class provides a set of static functions which are used to
// report a root or reference of a particular type. 
class Reporter : AllStatic {
 private:
  static JvmtiTagMap* _tag_map;
  static jvmtiHeapRootCallback _heap_root_callback;
  static jvmtiStackReferenceCallback _stack_ref_callback;
  static jvmtiObjectReferenceCallback _object_ref_callback;
  static void* _user_data;
  static GrowableArray<oop>* _visit_stack;
  static oop _last_referrer;		
  static jlong _last_referrer_tag;	

  static inline bool stack_ref_callback(jvmtiHeapRootKind root_kind, jlong thread_tag, 
					jint depth, jmethodID method, jint slot, oop o);

  static inline bool object_reference_callback(jvmtiObjectReferenceKind ref_kind,
					       oop referrer, oop referree, jint index);
 public:
  static void setup(JvmtiTagMap* tag_map, 
		    jvmtiHeapRootCallback heap_root_callback,
		    jvmtiStackReferenceCallback stack_ref_callback,
		    jvmtiObjectReferenceCallback object_ref_callback,
		    GrowableArray<oop>* visit_stack, 
                    void* user_data);

  static inline bool report_simple_root(jvmtiHeapRootKind, oop o);
  static inline bool report_jni_local_root(jlong thread_tag, oop o);
  static inline bool report_stack_ref_root(jlong thread_tag, jint depth, jmethodID method, 
					   jint slot, oop o);
  static inline bool report_array_element_reference(oop referrer, oop referree, jint index);
  static inline bool report_class_reference(oop referrer, oop referree);
  static inline bool report_class_loader_reference(oop referrer, oop referree);
  static inline bool report_signers_reference(oop referrer, oop referree);
  static inline bool report_protection_domain_reference(oop referrer, oop referree);
  static inline bool report_interface_reference(oop referrer, oop referree);
  static inline bool report_static_field_reference(oop referrer, oop referree, jint slot);
  static inline bool report_field_reference(oop referrer, oop referree, jint slot);
  static inline bool report_constant_pool_reference(oop referrer, oop referree, jint index);
};

JvmtiTagMap* Reporter::_tag_map;
jvmtiHeapRootCallback Reporter::_heap_root_callback;
jvmtiStackReferenceCallback Reporter::_stack_ref_callback;
jvmtiObjectReferenceCallback Reporter::_object_ref_callback;
void* Reporter::_user_data;
GrowableArray<oop>* Reporter::_visit_stack;
oop Reporter::_last_referrer;
jlong Reporter::_last_referrer_tag;


// setup/initialization
void Reporter::setup(JvmtiTagMap* tag_map,
		     jvmtiHeapRootCallback heap_root_callback,
		     jvmtiStackReferenceCallback stack_ref_callback,
		     jvmtiObjectReferenceCallback object_ref_callback,
		     GrowableArray<oop>* visit_stack, 
                     void* user_data)
{
  _tag_map = tag_map;
  _heap_root_callback = heap_root_callback;
  _stack_ref_callback = stack_ref_callback;
  _object_ref_callback = object_ref_callback;
  _visit_stack = visit_stack;
  _user_data = user_data;

  // cached referrer starts out as NULL at every iteration
  _last_referrer = NULL;
  _last_referrer_tag = 0;
}

// report a simple root to the profiler
inline bool Reporter::report_simple_root(jvmtiHeapRootKind kind, oop o) {
  assert(ServiceUtil::visible_oop(o), "checking");

  CallbackWrapper wrapper(_tag_map, o);
  jvmtiIterationControl control = (*_heap_root_callback)(kind, wrapper.klass_tag(), 
                                                         wrapper.obj_size(), wrapper.obj_tag_p(),
                                                         _user_data);
  if (control == JVMTI_ITERATION_CONTINUE && _object_ref_callback != NULL) {
    _visit_stack->push(o);
  }
  return control != JVMTI_ITERATION_ABORT;
}


// call the profiler's stack reference callback
inline bool Reporter::stack_ref_callback(jvmtiHeapRootKind root_kind, jlong thread_tag, 
					 jint depth, jmethodID method, jint slot, oop o)
{
  assert(ServiceUtil::visible_oop(o), "checking");

  CallbackWrapper wrapper(_tag_map, o);
  jvmtiIterationControl control = (*_stack_ref_callback)(root_kind, wrapper.klass_tag(), wrapper.obj_size(), 
                                                         wrapper.obj_tag_p(), thread_tag, depth, method, slot,
                                                         _user_data);
  if (control == JVMTI_ITERATION_CONTINUE && _object_ref_callback != NULL) {
    _visit_stack->push(o);
  }
  return control != JVMTI_ITERATION_ABORT;
}

// call the profiler's object reference callback
bool Reporter::object_reference_callback(jvmtiObjectReferenceKind ref_kind,
					 oop referrer, oop referree, jint index) {
  assert(ServiceUtil::visible_oop(referrer), "checking");
  assert(ServiceUtil::visible_oop(referree), "checking");

  // callback requires the referrer's tag. If it's the same referrer
  // as the last call then we use the cached value.
  jlong referrer_tag;
  if (referrer == _last_referrer) {
    referrer_tag = _last_referrer_tag;
  } else {
    referrer_tag = tag_for(_tag_map, klassOop_if_java_lang_Class(referrer));
  } 

  // do the callback  
  CallbackWrapper wrapper(_tag_map, referree);
  jvmtiIterationControl control = (*_object_ref_callback)(ref_kind, wrapper.klass_tag(), 
					                  wrapper.obj_size(), wrapper.obj_tag_p(), 
					                  referrer_tag, index,
                                                          _user_data);

  // record referrer and referrer tag. For self-references record the
  // tag value from the callback as this might differ from referrer_tag.
  _last_referrer = referrer;
  if (referrer == referree) {
    _last_referrer_tag = *wrapper.obj_tag_p();
  } else {
    _last_referrer_tag = referrer_tag;
  }

  if (control == JVMTI_ITERATION_CONTINUE && !ObjectMarker::visited(referree)) {
    _visit_stack->push(referree);
  }
  return control != JVMTI_ITERATION_ABORT;
}


// report a JNI local (root object) to the profiler
inline bool Reporter::report_jni_local_root(jlong thread_tag, oop o)

{
  return stack_ref_callback(JVMTI_HEAP_ROOT_JNI_LOCAL, thread_tag, -1, 
			    (jmethodID)-1, -1, o);
}

// report a local (stack reference, root object) to the profiler
inline bool Reporter::report_stack_ref_root(jlong thread_tag, jint depth, 
				            jmethodID method, jint slot, oop o) 
{
  return stack_ref_callback(JVMTI_HEAP_ROOT_STACK_LOCAL, thread_tag, depth,
			    method, slot, o);
}

// report an object referencing a class.
inline bool Reporter::report_class_reference(oop referrer, oop referree) {
  return object_reference_callback(JVMTI_REFERENCE_CLASS, referrer, referree, -1);
}

// report a class referencing its class loader.
inline bool Reporter::report_class_loader_reference(oop referrer, oop referree) {
  return object_reference_callback(JVMTI_REFERENCE_CLASS_LOADER, referrer, referree, -1);
}

// report a class referencing its signers.
inline bool Reporter::report_signers_reference(oop referrer, oop referree) {
  return object_reference_callback(JVMTI_REFERENCE_SIGNERS, referrer, referree, -1);
}

// report a class referencing its protection domain..
inline bool Reporter::report_protection_domain_reference(oop referrer, oop referree) {
  return object_reference_callback(JVMTI_REFERENCE_PROTECTION_DOMAIN, referrer, referree, -1);
}

// report a class referencing one of its interfaces.
inline bool Reporter::report_interface_reference(oop referrer, oop referree) {
  return object_reference_callback(JVMTI_REFERENCE_INTERFACE, referrer, referree, -1);
}

// report a class referencing one of its static fields.
inline bool Reporter::report_static_field_reference(oop referrer, oop referree, jint slot) {
  return object_reference_callback(JVMTI_REFERENCE_STATIC_FIELD, referrer, referree, slot);
}

// report an array referencing an element object
inline bool Reporter::report_array_element_reference(oop referrer, oop referree, jint index) {
  return object_reference_callback(JVMTI_REFERENCE_ARRAY_ELEMENT, referrer, referree, index);
}

// report an object referencing an instance field object
inline bool Reporter::report_field_reference(oop referrer, oop referree, jint slot) {
  return object_reference_callback(JVMTI_REFERENCE_FIELD, referrer, referree, slot);
}

// report an array referencing an element object
inline bool Reporter::report_constant_pool_reference(oop referrer, oop referree, jint index) {
  return object_reference_callback(JVMTI_REFERENCE_CONSTANT_POOL, referrer, referree, index);
}



// A supporting closure used to process simple roots
class SimpleRootsClosure : public OopClosure {
 private:  
  jvmtiHeapRootKind _kind;
  bool _continue;
 public:
  void set_kind(jvmtiHeapRootKind kind) {
    _kind = kind;
    _continue = true;
  }

  inline bool stopped() {
    return !_continue;
  }

  void do_oop(oop* obj_p) {   
    // iteration has terminated
    if (stopped()) {
      return;
    }

    // ignore null or deleted handles
    oop o = *obj_p;
    if (o == NULL || o == JNIHandles::deleted_handle()) {
      return;
    }     

    jvmtiHeapRootKind kind = _kind;

    // many roots are Klasses so we use the java mirror 
    if (o->is_klass()) {
      klassOop k = (klassOop)o;
      o = Klass::cast(k)->java_mirror();
    } else {

      // SystemDictionary::always_strong_oops_do reports the application
      // class loader as a root. We want this root to be reported as
      // a root kind of "OTHER" rather than "SYSTEM_CLASS".
      if (o->is_instance() && kind == JVMTI_HEAP_ROOT_SYSTEM_CLASS) {
	kind = JVMTI_HEAP_ROOT_OTHER;
      }
    }

    // some objects are ignored - in the case of simple
    // roots it's mostly symbolOops that we are skipping
    // here.
    if (!ServiceUtil::visible_oop(o)) {
      return;
    }     
 
    // invoke the callback
    _continue = Reporter::report_simple_root(kind, o);
  }
};

// A supporting closure used to process JNI locals
class JNILocalRootsClosure : public OopClosure {
 private:  
  jlong _thread_tag;
  bool _continue;
 public:
  void set_context(jlong thread_tag) {
    _thread_tag = thread_tag;
    _continue = true;
  }

  inline bool stopped() {
    return !_continue;
  }

  void do_oop(oop* obj_p) {  
    // iteration has terminated
    if (stopped()) {
      return;
    }

    // ignore null or deleted handles
    oop o = *obj_p;
    if (o == NULL || o == JNIHandles::deleted_handle()) {
      return;
    }

    if (!ServiceUtil::visible_oop(o)) {    
      return;
    }       

    // invoke the callback
    _continue = Reporter::report_jni_local_root(_thread_tag, o);
  }
};



// A VM operation to iterate over objects that are reachable from
// a set of roots or a given object.
//
// For IterateOverReachableObject the set of roots used is :-
//
// - All JNI global references
// - All inflated monitors
// - All classes loaded by the boot class loader (or all classes
//     in the event that class unloading is disabled)
// - All java threads
// - For each java thread then all locals and JNI local references
//      on the thread's execution stack
// - All visible/explainable objects from Universes::oops_do
//
class IterateOverReachableObjectOperation: public VM_Operation {
 private:
  JvmtiTagMap* _tag_map;
  oop _o;  
  bool _collecting_heap_roots;			    // are we collecting heap roots
  bool _collecting_stack_refs;			    // are we collect stack roots
  bool _following_object_refs;			    // are we reporting object references
  GrowableArray<oop>* _visit_stack;		    // the visit stack

  // iterate over the various object types
  inline bool iterate_over_array(oop o);
  inline bool iterate_over_type_array(oop o);
  inline bool iterate_over_class(klassOop o);
  inline bool iterate_over_object(oop o);

  // root collection
  inline bool collect_simple_roots();
  inline bool collect_stack_roots();
  inline bool collect_stack_roots(JavaThread* java_thread, JNILocalRootsClosure* blk);
 
  // visit an object
  inline bool visit(oop o);

  // initialization
  void init(JvmtiTagMap* tag_map, oop o,
	    jvmtiHeapRootCallback heap_root_callback, 
	    jvmtiStackReferenceCallback stack_ref_callback,
	    jvmtiObjectReferenceCallback object_ref_callback,
            void* user_data);

 public:        
  IterateOverReachableObjectOperation(JvmtiTagMap* tag_map, 
				      jvmtiHeapRootCallback heap_root_callback, 
				      jvmtiStackReferenceCallback stack_ref_callback,
				      jvmtiObjectReferenceCallback object_ref_callback, 
                                      void* user_data)
  {
    init(tag_map, NULL, heap_root_callback, stack_ref_callback, object_ref_callback, user_data);
  }

  IterateOverReachableObjectOperation(JvmtiTagMap* tag_map, 
                                      oop o, 
                                      jvmtiObjectReferenceCallback object_ref_callback,
                                      void* user_data) {
    init(tag_map, o, NULL, NULL, object_ref_callback, user_data);
  }

  ~IterateOverReachableObjectOperation() {
    if (_following_object_refs) {
      assert(_visit_stack != NULL, "checking");
      _visit_stack->clear_and_deallocate();
      FreeHeap(_visit_stack);    
      _visit_stack = NULL;
    }
  }

  void doit(); 

  void oops_do(OopClosure *f) {
    if (_o != NULL) {
      f->do_oop((oop*)&_o);    
    }
  }

  const char* name() const { return "JVMTI IterateOverReachableObjectOperation"; }
};

// initialize
void IterateOverReachableObjectOperation::init(JvmtiTagMap* tag_map, oop o,
                                               jvmtiHeapRootCallback heap_root_callback, 
                                               jvmtiStackReferenceCallback stack_ref_callback,
                                               jvmtiObjectReferenceCallback object_ref_callback, 
                                               void* user_data) 
{
  _tag_map = tag_map;
  _o = o;
  _collecting_heap_roots = (heap_root_callback != NULL);
  _collecting_stack_refs = (stack_ref_callback != NULL);
  _following_object_refs = (object_ref_callback != NULL);

  // if we are following references then setup the visit stack
  if (_following_object_refs) {
    _visit_stack = new (ResourceObj::C_HEAP) GrowableArray<oop>(4000,true);
  }  

  // initialize the Reporter with the callbacks
  Reporter::setup(_tag_map, heap_root_callback, stack_ref_callback, object_ref_callback, 
		  _visit_stack, user_data);
}

// an array references its class and has a reference to
// each element in the array
inline bool IterateOverReachableObjectOperation::iterate_over_array(oop o) {
  objArrayOop array = (objArrayOop)o;
  if (array->klass() == Universe::systemObjArrayKlassObj()) {
    // filtered out
    return true;
  }

  // array class
  oop k = Klass::cast(objArrayKlass::cast(array->klass())->element_klass())->java_mirror();

  // array reference to its class
  if (!Reporter::report_class_reference(o, k)) {
    return false;
  }

  // iterate over the array and report each reference to a
  // non-null element
  for (int index=0; index<array->length(); index++) {
    oop elem = array->obj_at(index);
    if (elem == NULL) {
      continue;	
    }

    // report the array reference o[index] = elem
    if (!Reporter::report_array_element_reference(o, elem, index)) {
      return false;
    }
  }
  return true;
}

// a type array references its class
inline bool IterateOverReachableObjectOperation::iterate_over_type_array(oop o) {
  klassOop k = o->klass();
  oop mirror = Klass::cast(k)->java_mirror();
  if (!Reporter::report_class_reference(o, mirror)) {
    return false;
  }
  return true;
}

// verify that a static oop field is in range
static inline bool verify_static_oop(instanceKlass* ik, oop* obj_p) {
  oop* start = ik->start_of_static_fields();
  oop* end = start + ik->static_oop_field_size();
  assert(end >= start, "sanity check");
  
  if (obj_p >= start && obj_p < end) {
    return true;
  } else {
    return false;
  }
}

// a class references its super class, interfaces, class loader, ...
// and finally its static fields
inline bool IterateOverReachableObjectOperation::iterate_over_class(klassOop k) {
  Klass* klass = klassOop(k)->klass_part();

  if (klass->oop_is_instance()) {         
    instanceKlass* ik = instanceKlass::cast(k);

    // ignore the class if it's has been initialized yet
    if (!ik->is_linked()) {
      return true;
    }

    // get the java mirror
    oop mirror = klass->java_mirror();
      
    // super
    klassOop java_super = ik->java_super();
    if (java_super != NULL) {
      oop super = Klass::cast(java_super)->java_mirror();
      if (!Reporter::report_class_reference(mirror, super)) {
        return false;
      }
    }

    // class loader	  
    oop cl = ik->class_loader();
    if (cl != NULL) {
      if (!Reporter::report_class_loader_reference(mirror, cl)) {
	return false;
      }
    }

    // protection domain
    oop pd = ik->protection_domain();
    if (pd != NULL) {
      if (!Reporter::report_protection_domain_reference(mirror, pd)) {
        return false;
      }
    }

    // signers
    oop signers = ik->signers();
    if (signers != NULL) {
      if (!Reporter::report_signers_reference(mirror, signers)) {
        return false;
      }
    }

    // resolved strings in the constant pool
    {
      const constantPoolOop pool = ik->constants();
      for (int i = 1; i < pool->length(); i++) {
        constantTag tag = pool->tag_at(i).value();
	if (tag.is_string()) {
	  oop o = pool->resolved_string_at(i);
	  if (!Reporter::report_constant_pool_reference(mirror, o, (jint)i)) {
	    return false;
	  }
        }        
      }
    }

    // interfaces
    objArrayOop interfaces = ik->local_interfaces();
    for (int i = 0; i < interfaces->length(); i++) {
      oop interf = Klass::cast((klassOop)interfaces->obj_at(i))->java_mirror();
      if (interf == NULL) {
        continue;
      }
      if (!Reporter::report_interface_reference(mirror, interf)) {
        return false;
      }
    }	 
    
    // static fields 
    //
    // We report the static fields declared in this class only. The
    // static fields in superclasses and interfaces will be reported
    // as being referenced from the superclass/interface. 
 
    HandleMark hm;
    instanceKlassHandle ikh = instanceKlassHandle(Thread::current(), k);   

    // need to know if this is a java.lang.Throwable
    bool is_throwable = klass->is_subclass_of(SystemDictionary::throwable_klass());

    // First we have to find out the total number of fields (both
    // static and instance). This includes all the fields in superclasses
    // and interfaces
    int field_count = 0;
    for (FieldStream fldc(ikh, false, false); !fldc.eos(); fldc.next()) {	      
      field_count++;      
    }
    int field_max = field_count-1;

    // Now iterate over the static field declared in this class only
    // (not superclasses/interfaces). The reason we don't need to
    // worry about superclass here is because the fields in this
    // class always appear "after" (higher field index) than fields
    // declared in the super classes.
    //
    int field_index = 0;   
    for (FieldStream fld(ikh, true, true); !fld.eos(); fld.next()) {	
      if (fld.access_flags().is_static()) {

	// Only interested in object or array fields. As per iterate_over_object
	// it would be possible to use SignatureIterator here but we find
	// that checking the first byte of the type is faster.	  
	int type = fld.signature()->byte_at(0);
        if (type =='L' || type == '[') {

	  int offset = fld.offset();

	  // get the address of the field
	  address addr = (address)k + offset;
	  oop* f = (oop*)addr;
	  assert(verify_static_oop(ik, f), "sanity check");

	  oop fld_o = *f;
	  if (fld_o != NULL) {
	    int slot = field_max-field_index;
	    // is the class extends java.lang.Throwable then we must adjust the
	    // slot number for all static fields that follow the backtrace field
	    // in the Throwable class.
	    if (is_throwable && slot > 1) {
	      slot--;
	    }
	    if (!Reporter::report_static_field_reference(mirror, fld_o, slot)) {
	      return false;
	    }
	  }	  
	}	  	 
      }	
      field_index++;      
    }

    return true;
  }

  // only reference from an object array class is its super class
  // (java.lang.Object?)
  if (klass->oop_is_objArray()) {
    oop mirror = klass->java_mirror();  
    objArrayKlass* ak = objArrayKlass::cast(k);
    klassOop java_super = ak->java_super();
    if (java_super != NULL) {
      oop super = Klass::cast(java_super)->java_mirror();
      if (!Reporter::report_class_reference(mirror, super)) {
        return false;
      }
    }
  } 

  return true;
}

// an object references a class and its instance fields
// (static fields are ignored here as we report these as 
// references from the class).
inline bool IterateOverReachableObjectOperation::iterate_over_object(oop o) {
  static GrowableArray<oop>* _fields;
  static GrowableArray<int>* _field_indices;
  HandleMark hm;

  // reference to the class 
  if (!Reporter::report_class_reference(o, Klass::cast(o->klass())->java_mirror())) {
    return false;
  }

  // FieldStream iterates over fields in reverse order so we collect
  // the fields in an array so that we can report them later with the
  // correct field index.
  if (_fields == NULL) {
    _fields = new (ResourceObj::C_HEAP) GrowableArray<oop>(50,true); 
    _field_indices = new (ResourceObj::C_HEAP) GrowableArray<int>(50,true);
  } else {
    _fields->clear();
    _field_indices->clear();
  }

  // The backtrace field of java.lang.Throwable cannot be reported
  // as it holds something that isn't a java.lang.Object - see 4446677.
  bool is_throwable = o->is_a(SystemDictionary::throwable_klass());
  

  // iterate over all the fields. ignore static fields here as they
  // will be reported as class fields. Also ignore fields that aren't
  // T_OBJECT or T_ARRAY fields.

  instanceKlassHandle k(o->klass());
  int field_index = -1; // start at 0
  for (FieldStream fld(k, false, false); !fld.eos(); fld.next()) {
    field_index++;

    if (!fld.access_flags().is_static()) {

      // It would be possible to use a SignatureIterator here but checking
      // the first byte seems to be faster.
      int type = fld.signature()->byte_at(0);
      if (type != 'L' && type != '[') {
	continue;
      }

      // ignore the java.lang.Throwable backtrace field
      if (is_throwable && fld.offset() == java_lang_Throwable::get_backtrace_offset()) {
	continue;
      }

      // the reference to the object or array
      address addr = (address)o + fld.offset();;
      oop* f = (oop*)addr;
      oop fld_o = *f;
      if (fld_o != NULL) {	    

 	// reflection code may have a reference to a klassOop.
	// - see sun.reflect.UnsafeStaticFieldAccessorImpl and sun.misc.Unsafe
	if (fld_o->is_klass()) {
	  klassOop k = (klassOop)fld_o;
	  fld_o = Klass::cast(k)->java_mirror();
        }

	_fields->append(fld_o);
	_field_indices->append(field_index);
      }
    } 
  }    


  // next iterate over the non-null instance fields and report them
  // to the profiler. As the fields were collected in reverse order we
  // generate the correct field index when reporting the field to the
  // profiler.
  int field_max = field_index;
  for (int i=0; i<_fields->length(); i++) {
    oop fld_o = _fields->at(i);
    field_index = _field_indices->at(i);
    jint slot = field_max-field_index;	
    if (is_throwable) {
      assert(slot != 1, "backtrace field is not reported");
      if (slot > 1) {
	slot--;	 
      }
    }
    if (!Reporter::report_field_reference(o, fld_o, slot)) {
      return false;      
    }
  }

  return true;
}


// collects all simple (non-stack) roots.
// if there's a heap root callback provided then the callback is
// invoked for each simple root.
// if an object reference callback is provided then all simple
// roots are pushed onto the marking stack so that they can be
// processed later
//
inline bool IterateOverReachableObjectOperation::collect_simple_roots() {
  SimpleRootsClosure blk;
      
  // JNI globals
  blk.set_kind(JVMTI_HEAP_ROOT_JNI_GLOBAL);
  JNIHandles::oops_do(&blk);  
  if (blk.stopped()) {
    return false;
  }

  // Preloaded classes and loader from the system dictionary
  blk.set_kind(JVMTI_HEAP_ROOT_SYSTEM_CLASS);
  SystemDictionary::always_strong_oops_do(&blk);
  if (blk.stopped()) {
    return false;
  }

  // Inflated monitors
  blk.set_kind(JVMTI_HEAP_ROOT_MONITOR);
  ObjectSynchronizer::oops_do(&blk);
  if (blk.stopped()) {
    return false;
  }

  // Threads
  for (JavaThread* thread = Threads::first(); thread != NULL ; thread = thread->next()) {
    oop threadObj = thread->threadObj();
    if (threadObj != NULL) {
      bool cont = Reporter::report_simple_root(JVMTI_HEAP_ROOT_THREAD, threadObj);     
      if (!cont) {
	return false;
      }
    }
  }

  // Other kinds of roots maintained by HotSpot
  // Many of these won't be visible but others (such as instances of important
  // exceptions) will be visible.
  blk.set_kind(JVMTI_HEAP_ROOT_OTHER);
  Universe::oops_do(&blk);

  return true;
}

// Walk the stack of a given thread and find all references (locals
// and JNI calls) and report these as stack references
inline bool IterateOverReachableObjectOperation::collect_stack_roots(JavaThread* java_thread, 
								     JNILocalRootsClosure* blk) 
{
  oop threadObj = java_thread->threadObj();
  assert(threadObj != NULL, "sanity check");
  
  // only need to get the thread's tag once per thread
  jlong thread_tag = tag_for(_tag_map, threadObj);

  // JNI locals for the top frame
  blk->set_context(thread_tag);
  java_thread->active_handles()->oops_do(blk);

  if (java_thread->has_last_Java_frame()) {

    // vframes are resource allocated
    Thread* current_thread = Thread::current(); 
    ResourceMark rm(current_thread);
    HandleMark hm(current_thread);

    RegisterMap reg_map(java_thread);   
    frame f = java_thread->last_frame();
    vframe* vf = vframe::new_vframe(&f, &reg_map, java_thread);
   
    int depth = 0;
    while (vf != NULL) {
      if (vf->is_java_frame()) {

	// java frame (interpreted, compiled, ...)
	javaVFrame *jvf = javaVFrame::cast(vf);

	jmethodID method = jvf->method()->jmethod_id();
	if (!(jvf->method()->is_native())) {         	
	  StackValueCollection* locals = jvf->locals();
	  for (int slot=0; slot<locals->size(); slot++) {
	    if (locals->at(slot)->type() == T_OBJECT) {
	      oop o = locals->obj_at(slot)();
	      if (o == NULL) {
	        continue;
	      }	    

	      // stack reference	    
	      if (!Reporter::report_stack_ref_root(thread_tag, depth, method, 
						   slot, o))
	      {
	        return false;
	      }
	    }
	  }
	}

	// depth relates to the java frames only 
	depth++;
      } else {

	// externalVFrame - if it's an entry frame then report any JNI locals
	// as roots
	frame* fr = vf->frame_pointer();
        assert(fr != NULL, "sanity check");
        if (fr->is_entry_frame()) {
	  blk->set_context(thread_tag);
          fr->entry_frame_call_wrapper()->handles()->oops_do(blk);
	}
      }

      vf = vf->sender();
    }  
  }
  return true;
}


// collects all stack roots - for each thread it walks the execution
// stack to find all references and local JNI refs.
inline bool IterateOverReachableObjectOperation::collect_stack_roots() {
  JNILocalRootsClosure blk;
  for (JavaThread* thread = Threads::first(); thread != NULL ; thread = thread->next()) {
    oop threadObj = thread->threadObj();
    if (threadObj != NULL) {     
      if (!collect_stack_roots(thread, &blk)) {
	return false;
      }
    }
  }
  return true;
}

// visit an object 
// first mark the object as visited
// second get all the outbound references from this object (in other words, all
// the objects referenced by this object).
//
bool IterateOverReachableObjectOperation::visit(oop o) { 
  // mark object as visited
  assert(!ObjectMarker::visited(o), "can't visit same object more than once");
  ObjectMarker::mark(o);

  // instance
  if (o->is_instance()) {
    if (o->klass() == SystemDictionary::class_klass()) {
      o = klassOop_if_java_lang_Class(o);
      if (o->is_klass()) {
	// a java.lang.Class	
        return iterate_over_class(klassOop(o));
      }
    } else {
      return iterate_over_object(o);
    }
  }

  // object array
  if (o->is_objArray()) {  
    return iterate_over_array(o);
  }

  // type array
  if (o->is_typeArray()) {
    return iterate_over_type_array(o);
  }  

  return true;
}

void IterateOverReachableObjectOperation::doit() {
  ResourceMark rm;
  ObjectMarkerController marker;

  if (_following_object_refs) {
    guarantee(_visit_stack->is_empty(), "visit stack must be empty");

    // if a root object has been provided then seed the visit
    // stack with the object
    if (_o != NULL) {
      _visit_stack->push(_o);
    }
  }

  // if the heap root callback is specified then heap roots
  // are pushed onto the stack and the callback invoked for
  // each root.
  if (_collecting_heap_roots) {
    if (!collect_simple_roots()) {
      // callback terminated the iteration
      return;
    }
  }

  // if stack references callback is specified then stack roots
  // are pushed onto the stack and the callback invoked for each
  // stack root.
  if (_collecting_stack_refs) {
    if (!collect_stack_roots()) {
      // callback terminated the iteration
      return;
    }
  }

  // object references required
  if (_following_object_refs) {

    // visit each object until all reachable objects have been
    // visited or the callback asked to terminate the iteration.
    while (!_visit_stack->is_empty()) {
      oop o = _visit_stack->pop();
      if (!ObjectMarker::visited(o)) {
	if (!visit(o)) {
	  break;
	}
      }
    }
  } 
}

// iterate over all objects that are reachable from a set of roots
void JvmtiTagMap::iterate_over_reachable_objects(jvmtiHeapRootCallback heap_root_callback, 
                                                 jvmtiStackReferenceCallback stack_ref_callback,
                                                 jvmtiObjectReferenceCallback object_ref_callback, 
                                                 void* user_data)
{  
  MutexLocker ml(Heap_lock);
  IterateOverReachableObjectOperation op(this, heap_root_callback, stack_ref_callback, 
  					 object_ref_callback, user_data);
  VMThread::execute(&op); 
}

// iterate over all objects that are reachable from a given object
void JvmtiTagMap::iterate_over_objects_reachable_from_object(jobject object, 
                                                             jvmtiObjectReferenceCallback object_reference_callback,
                                                             void* user_data)
{
  MutexLocker ml(Heap_lock);
  oop o = JNIHandles::resolve(object);
  IterateOverReachableObjectOperation op(this, o, object_reference_callback, user_data);
  VMThread::execute(&op);
}


// called post-GC 
// - for each JVMTI environment with an object tag map, call its rehash
// function to re-sync with the new object locations.
void JvmtiTagMap::gc_complete(bool full) { 
  assert(Thread::current()->is_VM_thread(), "should be VM thread");
  int env_count = JvmtiEnv::env_count();
  if (env_count > 0) {
    // re-obtain the memory region for the young generation (might
    // changed due to adaptive resizing policy)
    get_young_generation();  

    TraceTime t(full ? "JVMTI Full Rehash" : "JVMTI Rehash", TraceJVMTIObjectTagging); 
    for (int i = 0; i < env_count; ++i) {
      JvmtiEnv *env = JvmtiEnv::env_at(i);
      JvmtiTagMap* tag_map = ((JvmtiEnvBase *)env)->tag_map();
      if (tag_map != NULL) {
        tag_map->rehash(full);
      }
    }
  }
}



// For each entry in the hashmap :- 
//
// 1. resolve the JNI weak reference
//
// 2. If it resolves to NULL it means the object has been freed so the entry
//    is removed, the weak reference destroyed, and the object free event is
//    posted (if enabled).
//
// 3. If the weak reference resolves to an object then we re-hash the object
//    to see if it has moved or has been promoted (from the young to the old
//    generation for example). 
//   
void JvmtiTagMap::rehash(bool full) {

  // does this environment have the OBJECT_FREE event enabled
  bool post_object_free = env()->is_enabled(JVMTI_EVENT_OBJECT_FREE);

  // counters used for trace message
  int freed = 0;
  int moved = 0;
  int promoted = 0;  

  // we assume there are two hashmaps - one for the young generation
  // and the other for all other spaces.
  assert(_n_hashmaps == 2, "not implemented");
  JvmtiTagHashmap* young_hashmap = _hashmap[0];
  JvmtiTagHashmap* other_hashmap = _hashmap[1];

  // when re-hashing the hashmap corresponding to the young generation we
  // collect the entries corresponding to objects that have been promoted.
  JvmtiTagHashmapEntry* promoted_entries = NULL;

  for (int i=0; i<_n_hashmaps; i++) {
    JvmtiTagHashmap* hashmap = _hashmap[i];

    // if this wasn't a full GC then we only need to re-hash the first hashmap
    if (!full && i>0) {
      break;
    }
    
    // if the hashmap is empty then we can skip it
    if (hashmap->_entry_count == 0) {
      continue;
    }


    // now iterate through each entry in the table

    JvmtiTagHashmapEntry** table = hashmap->table();
    int size = hashmap->size();

    for (int pos=0; pos<size; pos++) {
      JvmtiTagHashmapEntry* entry = table[pos];
      JvmtiTagHashmapEntry* prev = NULL;

      while (entry != NULL) {
        JvmtiTagHashmapEntry* next = entry->next();

        jweak ref = entry->object();
        oop oop = JNIHandles::resolve(ref);	

        // has object been GC'ed
        if (oop == NULL) {
	  // grab the tag 
	  jlong tag = entry->tag();
	  guarantee(tag != 0, "checking");

	  // remove GC'ed entry from hashmap and return the
	  // entry to the free list
	  hashmap->remove(prev, pos, entry);
	  destroy_entry(entry);

	  // destroy the weak ref
          JNIHandles::destroy_weak_global(ref);

	  // post the event to the profiler
          if (post_object_free) {
            JvmtiExport::post_object_free(env(), tag);
	  }      

	  freed++;
	  entry = next;
	  continue;
	}

	// if this is the young hashmap then the object is either promoted
	// or moved.
	// if this is the other hashmap then the object is moved.

	bool same_gen;
	if (i == 0) {
	  assert(hashmap == young_hashmap, "checking");
	  same_gen = is_in_young(oop);
	} else {
	  same_gen = true;
	}
  

	if (same_gen) {     
	  // if the object has moved then re-hash it and move its
	  // entry to its new location.
	  unsigned int new_pos = JvmtiTagHashmap::hash(oop, size);
	  if (new_pos != (unsigned int)pos) {
	    if (prev == NULL) {
	      table[pos] = next;
	    } else {
	      prev->set_next(next);
	    }
	    entry->set_next(table[new_pos]);
	    table[new_pos] = entry;
	    moved++; 
	  } else {
	    // object didn't move
	    prev = entry;
	  }
	} else {	    
	  // object has been promoted so remove the entry from the
	  // young hashmap
	  assert(hashmap == young_hashmap, "checking");
	  hashmap->remove(prev, pos, entry);
	  
	  // move the entry to the promoted list 
	  entry->set_next(promoted_entries);
	  promoted_entries = entry;	  	 
	}     

	entry = next;
      }
    }
  }  

  
  // add the entries, corresponding to the promoted objects, to the
  // other hashmap.
  JvmtiTagHashmapEntry* entry = promoted_entries;
  while (entry != NULL) {
    oop o = JNIHandles::resolve(entry->object());
    assert(hashmap_for(o) == other_hashmap, "checking");
    JvmtiTagHashmapEntry* next = entry->next();
    other_hashmap->add(o, entry);
    entry = next;
    promoted++;
  }

  // stats
  if (TraceJVMTIObjectTagging) {
    int total_moves = promoted + moved;

    int post_total = 0;
    for (int i=0; i<_n_hashmaps; i++) {
      post_total += _hashmap[i]->_entry_count;
    }
    int pre_total = post_total + freed;
   
    tty->print("(%d->%d, %d freed, %d promoted, %d total moves)", 
	pre_total, post_total, freed, promoted, total_moves);     
  }
}
