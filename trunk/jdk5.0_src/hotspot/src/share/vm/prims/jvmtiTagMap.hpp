#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiTagMap.hpp	1.12 03/12/23 16:43:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// JvmtiTagMap 

#ifndef _JAVA_JVMTI_TAG_MAP_H_
#define _JAVA_JVMTI_TAG_MAP_H_

// forward references
class JvmtiTagHashmap;
class JvmtiTagHashmapEntry;
class JvmtiTagHashmapEntryClosure;

class JvmtiTagMap :  public CHeapObj {
 private:

  enum{	    
    _n_hashmaps = 2				    // encapsulates 2 hashmaps
  };

  // memory region for young generation
  static MemRegion _young_gen;
  static void get_young_generation();

  JvmtiEnv*		_env;			    // the jvmti environment
  Mutex			_lock;			    // lock for this tag map
  JvmtiTagHashmap*	_hashmap[_n_hashmaps];	    // the hashmaps 
  JvmtiTagHashmapEntry* _free_entries;		    // free list for this environment

  // create a tag map
  JvmtiTagMap(JvmtiEnv* env);				  

  // accessors
  inline Mutex* lock()			    { return &_lock; }
  inline JvmtiEnv* env() const		    { return _env; }

  // rehash this tag map
  void rehash(bool full);

  // indicates if the object is in the young generation
  static bool is_in_young(oop o);

  // iterate over all entries in this tag map
  void entry_iterate(JvmtiTagHashmapEntryClosure* closure);
 
 public:

  // indicates if this tag map is locked
  bool is_locked()			    { return lock()->is_locked(); }  

  // return the appropriate hashmap for a given object
  JvmtiTagHashmap* hashmap_for(oop o);

  // create/destroy entries
  JvmtiTagHashmapEntry* create_entry(jweak ref, jlong tag);
  void destroy_entry(JvmtiTagHashmapEntry* entry);

  // return tag for the given environment
  static JvmtiTagMap* tag_map_for(JvmtiEnv* env);

  // destroy tag map
  ~JvmtiTagMap();

  // set/get tag
  void set_tag(jobject obj, jlong tag);
  jlong get_tag(jobject obj);

  // iteration functions
  void iterate_over_heap(jvmtiHeapObjectFilter object_filter, 
			 jvmtiHeapObjectCallback heap_object_callback, 
                         void* user_data); 

  void iterate_over_instances_of_class(klassOop k_oop, jvmtiHeapObjectFilter object_filter, 
				       jvmtiHeapObjectCallback heap_object_callback, 
                                       void* user_data); 

  void iterate_over_reachable_objects(jvmtiHeapRootCallback heap_root_callback, 
				      jvmtiStackReferenceCallback stack_ref_callback, 
				      jvmtiObjectReferenceCallback object_ref_callback, 
                                      void* user_data);

  void iterate_over_objects_reachable_from_object(jobject object, 
						  jvmtiObjectReferenceCallback object_reference_callback,
                                                  void* user_data);

  // get tagged objects
  jvmtiError get_objects_with_tags(const jlong* tags, jint count, 
				   jint* count_ptr, jobject** object_result_ptr, 
				   jlong** tag_result_ptr);

  // call post-GC to rehash the tag maps.
  static void gc_complete(bool full);
};

#endif   /* _JAVA_JVMTI_TAG_MAP_H_ */

