#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmtiGetLoadedClasses.cpp	1.5 03/12/23 16:43:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


# include "incls/_precompiled.incl"
# include "incls/_jvmtiGetLoadedClasses.cpp.incl"


// The closure for GetLoadedClasses and GetClassLoaderClasses
class JvmtiGetLoadedClassesClosure : public StackObj {
  // Since the SystemDictionary::classes_do callback 
  // doesn't pass a closureData pointer, 
  // we use a thread-local slot to hold a pointer to 
  // a stack allocated instance of this structure.
 private:
  jobject _initiatingLoader;
  int     _count;
  Handle* _list;
  int     _index;

 private:
  // Getting and setting the thread local pointer
  static JvmtiGetLoadedClassesClosure* get_this() {
    JvmtiGetLoadedClassesClosure* result = NULL;
    JavaThread* thread = JavaThread::current();
    result = thread->get_jvmti_get_loaded_classes_closure();
    return result;
  }
  static void set_this(JvmtiGetLoadedClassesClosure* that) {
    JavaThread* thread = JavaThread::current();
    thread->set_jvmti_get_loaded_classes_closure(that);
  }

 public:
  // Constructor/Destructor
  JvmtiGetLoadedClassesClosure() {
    JvmtiGetLoadedClassesClosure* that = get_this();
    assert(that == NULL, "JvmtiGetLoadedClassesClosure in use");
    _initiatingLoader = NULL;
    _count = 0;
    _list = NULL;
    _index = 0;
    set_this(this);
  }

  JvmtiGetLoadedClassesClosure(jobject initiatingLoader) {
    JvmtiGetLoadedClassesClosure* that = get_this();
    assert(that == NULL, "JvmtiGetLoadedClassesClosure in use");
    _initiatingLoader = initiatingLoader;
    _count = 0;
    _list = NULL;
    _index = 0;
    set_this(this);
  }

  ~JvmtiGetLoadedClassesClosure() {
    JvmtiGetLoadedClassesClosure* that = get_this();
    assert(that != NULL, "JvmtiGetLoadedClassesClosure not found");
    set_this(NULL);
    _initiatingLoader = NULL;
    _count = 0;
    if (_list != NULL) {
      FreeHeap(_list);
      _list = NULL;
    }
    _index = 0;
  }

  // Accessors.
  jobject get_initiatingLoader() {
    return _initiatingLoader;
  }

  int get_count() {
    return _count;
  }

  void set_count(int value) {
    _count = value;
  }

  Handle* get_list() {
    return _list;
  }

  void set_list(Handle* value) {
    _list = value;
  }

  int get_index() {
    return _index;
  }

  void set_index(int value) {
    _index = value;
  }

  Handle get_element(int index) {
    if ((_list != NULL) && (index < _count)) {
      return _list[index];
    } else {
      assert(false, "empty get_element");
      return Handle();
    }
  }

  void set_element(int index, Handle value) {
    if ((_list != NULL) && (index < _count)) {
      _list[index] = value;
    } else {
      assert(false, "bad set_element");
    }
  }

  // Other predicates
  bool available() {
    return (_list != NULL);
  }

#ifdef ASSERT
  // For debugging.
  void check(int limit) {
    for (int i = 0; i < limit; i += 1) {
      assert(Universe::heap()->is_in(get_element(i)()), "check fails");
    }
  }
#endif

  // Public methods that get called within the scope of the closure
  void allocate() {
    _list = NEW_C_HEAP_ARRAY(Handle, _count);
    assert(_list != NULL, "Out of memory");
    if (_list == NULL) {
      _count = 0;
    }
  }

  void extract(JvmtiEnv *env, jclass* result) {
    for (int index = 0; index < _count; index += 1) {
      result[index] = (jclass) env->jni_reference(get_element(index));
    }
  }

  // Finally, the static methods that are the callbacks
  static void increment(klassOop k) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (that->get_initiatingLoader() == NULL) {
      for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
        that->set_count(that->get_count() + 1);
      }
    } else if (k != NULL) {
      // if initiating loader not null, just include the instance with 1 dimension
      that->set_count(that->get_count() + 1);
    }
  }

  static void increment_with_loader(klassOop k, oop loader) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
      for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
        that->set_count(that->get_count() + 1);
      }
    }
  }

  static void prim_array_increment_with_loader(klassOop array, oop loader) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
      that->set_count(that->get_count() + 1);
    }
  }

  static void add(klassOop k) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (that->get_initiatingLoader() == NULL) {
        for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
          oop mirror = Klass::cast(l)->java_mirror();
          that->set_element(that->get_index(), mirror);
          that->set_index(that->get_index() + 1);
        }
      } else if (k != NULL) {
        // if initiating loader not null, just include the instance with 1 dimension
        oop mirror = Klass::cast(k)->java_mirror();
        that->set_element(that->get_index(), mirror);
        that->set_index(that->get_index() + 1);
      }        
    }
  }

  static void add_with_loader(klassOop k, oop loader) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
        for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
          oop mirror = Klass::cast(l)->java_mirror();
          that->set_element(that->get_index(), mirror);
          that->set_index(that->get_index() + 1);
        }
      }
    }
  }

  static void prim_array_add_with_loader(klassOop array, oop loader) {
    JvmtiGetLoadedClassesClosure* that = JvmtiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
        oop mirror = Klass::cast(array)->java_mirror();
        that->set_element(that->get_index(), mirror);
        that->set_index(that->get_index() + 1);
      }
    }
  }

};


jvmtiError
JvmtiGetLoadedClasses::getLoadedClasses(JvmtiEnv *env, jint* classCountPtr, jclass** classesPtr) {
  // Since SystemDictionary::classes_do only takes a function pointer 
  // and doesn't call back with a closure data pointer, 
  // we can only pass static methods.

  JvmtiGetLoadedClassesClosure closure;
  {
    // For consistency of the loaded classes, grab the SystemDictionary lock
    MutexLocker sd_mutex(SystemDictionary_lock);
    // First, count the classes
    SystemDictionary::classes_do(&JvmtiGetLoadedClassesClosure::increment);
    Universe::basic_type_classes_do(&JvmtiGetLoadedClassesClosure::increment);
    // Next, fill in the classes
    closure.allocate();
    SystemDictionary::classes_do(&JvmtiGetLoadedClassesClosure::add);
    Universe::basic_type_classes_do(&JvmtiGetLoadedClassesClosure::add);
    // Drop the SystemDictionary_lock, so the results could be wrong from here,
    // but we still have a snapshot.
  }
  // Post results
  jclass* result_list;
  jvmtiError err = env->Allocate(closure.get_count() * sizeof(jclass), 
                                 (unsigned char**)&result_list);
  if (err != JVMTI_ERROR_NONE) {
    return err;
  }
  closure.extract(env, result_list);
  *classCountPtr = closure.get_count();
  *classesPtr = result_list;
  return JVMTI_ERROR_NONE;    
} 

jvmtiError
JvmtiGetLoadedClasses::getClassLoaderClasses(JvmtiEnv *env, jobject initiatingLoader, 
                                             jint* classCountPtr, jclass** classesPtr) {
  // Since SystemDictionary::classes_do only takes a function pointer 
  // and doesn't call back with a closure data pointer, 
  // we can only pass static methods.
  JvmtiGetLoadedClassesClosure closure(initiatingLoader);
  {
    // For consistency of the loaded classes, grab the SystemDictionary lock
    MutexLocker sd_mutex(SystemDictionary_lock);
    // First, count the classes
    // a. SystemDictionary::classes_do doesn't include arrays of primitive types (any dimensions)
    SystemDictionary::classes_do(&JvmtiGetLoadedClassesClosure::increment_with_loader);
    // b. Count arrays of primitive types
    SystemDictionary::prim_array_classes_do(&JvmtiGetLoadedClassesClosure::prim_array_increment_with_loader);
    // c. single-dimensional array of primitive types created by newarray instruction
    //    do not require resolution; thus they are not added in system dictionary.
    //    For simplicity, add single-dimensional array of all primitive types.
    Universe::basic_type_classes_do(&JvmtiGetLoadedClassesClosure::increment);
    // Next, fill in the classes
    closure.allocate();
    SystemDictionary::classes_do(&JvmtiGetLoadedClassesClosure::add_with_loader);
    SystemDictionary::prim_array_classes_do(&JvmtiGetLoadedClassesClosure::prim_array_add_with_loader);
    Universe::basic_type_classes_do(&JvmtiGetLoadedClassesClosure::add);
    // Drop the SystemDictionary_lock, so the results could be wrong from here,
    // but we still have a snapshot.
  }
  // Post results
  jclass* result_list;
  jvmtiError err = env->Allocate(closure.get_count() * sizeof(jclass), 
                                 (unsigned char**)&result_list);
  if (err != JVMTI_ERROR_NONE) {
    return err;
  }
  closure.extract(env, result_list);
  *classCountPtr = closure.get_count();
  *classesPtr = result_list;
  return JVMTI_ERROR_NONE;    
}
