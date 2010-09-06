
#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiEnvBase.hpp	1.39 04/03/18 16:52:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMTIENVBASE_H_
#define _JAVA_JVMTIENVBASE_H_

//
// Forward Declarations
//

class JvmtiEnv;
class JvmtiThreadState;
class JvmtiRawMonitor; // for jvmtiEnv.hpp
class JvmtiEventControllerPrivate;
class JvmtiTagMap;



// One JvmtiEnv object is created per jvmti/jvmdi attachment;
// done via JNI GetEnv() call. Multiple attachments are
// allowed in jvmti and in case of jvmdi only one attachment
// works so only one object is created for jvmdi.

class JvmtiEnvBase : public CHeapObj {
    
protected:
    
  JVMDI_AllocHook _ahook;             // JVMDI use only
  JVMDI_DeallocHook _dhook;           // JVMDI use only
  jvmtiEnv _jvmti_external;
  const void *_env_local_storage;     // per env agent allocated data.
  jvmtiEventCallbacks _event_callbacks;
  jvmtiExtEventCallbacks _ext_event_callbacks;
  bool _jvmdi;
  JvmtiTagMap* _tag_map;
     
 private:
  static bool              _globally_initialized;
  static jvmtiPhase        _phase;

 public:    

  static jvmtiPhase  get_phase()                    { return _phase; }
  static void  set_phase(jvmtiPhase phase)          { _phase = phase; }
  static bool is_vm_live()                          { return _phase == JVMTI_PHASE_LIVE; } 

private:
    
  static GrowableArray<JvmtiEnvBase*> *_environments;

  enum {
      JVMTI_MAGIC = 0x71EE,
      JVMDI_MAGIC = 0xD1EE,
      BAD_MAGIC   = 0xDEAD
  };

  jint _magic;
  jint _index;

  JvmtiEnvEventEnable _env_event_enable;

  jvmtiCapabilities _current_capabilities;
        
protected:
  JvmtiEnvBase(bool jvmdi);
  ~JvmtiEnvBase();

  static JvmtiEnv *_jvmti_env_for_jvmdi;

private:
  friend class JvmtiEventControllerPrivate;
  void initialize();
  void dispose();
  void set_event_callbacks(const jvmtiEventCallbacks* callbacks, jint size_of_callbacks);
  static void globally_initialize();

public:
  bool is_valid()       { return _magic == JVMTI_MAGIC; }
  bool is_valid_jvmdi() { return _magic == JVMDI_MAGIC; }

  static ByteSize jvmti_external_offset() {
    return byte_offset_of(JvmtiEnvBase, _jvmti_external); 
  };

  static JvmtiEnv* JvmtiEnv_from_jvmti_env(jvmtiEnv *env) {
    return (JvmtiEnv*)((intptr_t)env - in_bytes(jvmti_external_offset()));
  };

  static JvmtiEnv *jvmti_env_for_jvmdi()  { 
    return _jvmti_env_for_jvmdi; 
  }

  bool is_jvmdi() { 
    return _jvmdi; 
  }

  jvmtiCapabilities *get_capabilities()  { return &_current_capabilities; }

  static int env_count() {
    return _environments->length();
  }

  static JvmtiEnv* env_at(int index) {
    return (JvmtiEnv*)_environments->at(index);
  }

  int index() { return _index; }

  JvmtiEnvEventEnable *env_event_enable() { 
    return &_env_event_enable; 
  }

  jvmtiError jvmdi_allocate(jlong size, unsigned char** mem_ptr);
  jvmtiError jvmdi_deallocate(unsigned char* mem);

  jvmtiError allocate(jlong size, unsigned char** mem_ptr) {
    if (size < 0) {
      return JVMTI_ERROR_ILLEGAL_ARGUMENT;
    }
    if (is_jvmdi()) {
      return jvmdi_allocate(size, mem_ptr);
    } else {
      if (size == 0) {
        *mem_ptr = NULL;
      } else {
        *mem_ptr = (unsigned char *)os::malloc((size_t)size); 
        if (*mem_ptr == NULL) {
          return JVMTI_ERROR_OUT_OF_MEMORY;
        }
      }
    }
    return JVMTI_ERROR_NONE;
  }

  jvmtiError deallocate(unsigned char* mem) {
    if (is_jvmdi()) {
      return jvmdi_deallocate(mem);
    } else {
      if (mem != NULL) {
        os::free(mem);
      }
      return JVMTI_ERROR_NONE;
    }
  }


  // Memory functions 
  unsigned char* jvmtiMalloc(jlong size);  // don't use this - call allocate

  // method to create global handle for jvmdi and local handle
  // for jvmti.    
  jobject jni_reference(Handle hndl) {
    if (_jvmdi) {
      return JNIHandles::make_global(hndl);
    } else {
      return JNIHandles::make_local(hndl());  
    }
  }

  // method to destroy global handle for jvmdi and local handle
  // for jvmti.    
  void destroy_jni_reference(jobject jobj) {
    if (_jvmdi) {
      JNIHandles::destroy_global(jobj);
    } else {
      JNIHandles::destroy_local(jobj);  
    }
  }
    
    
  jvmtiEnv* jvmti_external() { return &_jvmti_external; };


// Event Dispatch

  bool has_callback(jvmtiEvent event_type) {
    assert(event_type >= JVMTI_MIN_EVENT_TYPE_VAL && 
	   event_type <= JVMTI_MAX_EVENT_TYPE_VAL, "checking");
    return ((void**)&_event_callbacks)[event_type-JVMTI_MIN_EVENT_TYPE_VAL] != NULL;
  }

  jvmtiEventCallbacks* callbacks() {
    return &_event_callbacks;
  }

  jvmtiExtEventCallbacks* ext_callbacks() {
    return &_ext_event_callbacks;
  }

  void set_tag_map(JvmtiTagMap* tag_map) {
    _tag_map = tag_map;
  }

  JvmtiTagMap* tag_map() {
    return _tag_map;
  }


  // return true if event is enabled globally or for any thread
  // True only if there is a callback for it.
  bool is_enabled(jvmtiEvent event_type) { 
    return _env_event_enable.is_enabled(event_type); 
  }

// Random Utilities

  // convert to a jni jmethodID from a non-null methodOop and return the class oop
  oop get_jvmdi_method_and_class(jmethodID mid, jmethodID *mid_p);

 protected:
  // helper methods for creating arrays of global JNI Handles from local Handles
  // allocated into environment specific storage
  jobject * new_jobjectArray(int length, Handle *handles);
  jthread * new_jthreadArray(int length, Handle *handles);
  jthreadGroup * new_jthreadGroupArray(int length, Handle *handles);

  // convert from JNIHandle to JavaThread *
  JavaThread  * get_JavaThread(jthread jni_thread);

  // convert to a jni jclass from a non-null klassOop
  jclass get_jni_class_non_null(klassOop k);
    
  void update_klass_field_access_flag(fieldDescriptor *fd);

  jint count_locked_objects(JavaThread *java_thread, Handle hobj);
  void get_locked_objects_in_frame(JavaThread* java_thread, javaVFrame *jvf,
                                   GrowableArray<jobject>* locked_object_list);

  vframe* vframeFor(JavaThread* java_thread, jint depth);

 public:
  // get a field descriptor for the specified class and field
  static bool get_field_descriptor(klassOop k, jfieldID field, fieldDescriptor* fd);
  // test for suspend - most (all?) of these should go away
  static bool is_thread_fully_suspended(JavaThread *thread,
                                        bool wait_for_suspend,
                                        uint32_t *bits);


  // JVMTI API helper functions which are called at safepoint or thread is suspended.
  jvmtiError JvmtiEnvBase::get_frame_count(JvmtiThreadState *state, jint *count_ptr);
  jvmtiError JvmtiEnvBase::get_frame_location(JavaThread* java_thread, jint depth, 
                                              jmethodID* method_ptr, jlocation* location_ptr);
  jvmtiError JvmtiEnvBase::get_object_monitor_usage(jobject object, jvmtiMonitorUsage* info_ptr);
  jvmtiError JvmtiEnvBase::get_stack_trace(JavaThread *java_thread, 
                                           jint stack_depth, jint max_count,
                                           jvmtiFrameInfo* frame_buffer, jint* count_ptr);
  jvmtiError JvmtiEnvBase::get_owned_monitor_info(JavaThread *java_thread,
                                                         jint* count_ptr,
                                                         jobject** owned_monitors_ptr);
  jvmtiError JvmtiEnvBase::get_current_contended_monitor(JavaThread *java_thread,
                                                         jobject *monitor_ptr);

};

    
// VM operation to get owned monitors.
class VM_GetOwnedMonitorInfo : public VM_Operation {
private:
  JvmtiEnv *_env;
  JavaThread *_java_thread;
  jint *_count_ptr;
  jobject **_owned_monitors_ptr;
  jvmtiError _result;

public:
  VM_GetOwnedMonitorInfo(JvmtiEnv *env, JavaThread *java_thread, jint *count_ptr,  jobject **mon_ptr) {
    _env = env;
    _java_thread = java_thread;
    _count_ptr = count_ptr;
    _owned_monitors_ptr = mon_ptr;
  }
  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase*) _env)->get_owned_monitor_info(_java_thread, _count_ptr, _owned_monitors_ptr);
  }
  const char* name() const { return "get owned monitor info"; }      
};

    
// VM operation to get object monitor usage.
class VM_GetObjectMonitorUsage : public VM_Operation {
private:
  JvmtiEnv *_env;
  jobject _object;
  jvmtiMonitorUsage* _info_ptr;
  jvmtiError _result;

public:
  VM_GetObjectMonitorUsage(JvmtiEnv *env, jobject object, jvmtiMonitorUsage* info_ptr) {
    _env = env;
    _object = object;
    _info_ptr = info_ptr;
  }
  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase*) _env)->get_object_monitor_usage(_object, _info_ptr);
  }
  const char* name() const { return "get object monitor usage"; }      
};

// VM operation to get current contended monitor.
class VM_GetCurrentContendedMonitor : public VM_Operation {
private:
  JvmtiEnv *_env;
  JavaThread *_java_thread;
  jobject *_owned_monitor_ptr;
  jvmtiError _result;

public:
  VM_GetCurrentContendedMonitor(JvmtiEnv *env, JavaThread *java_thread, jobject *mon_ptr) {
    _env = env;
    _java_thread = java_thread;
    _owned_monitor_ptr = mon_ptr;
  }
  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase *)_env)->get_current_contended_monitor(_java_thread,_owned_monitor_ptr);
  }
  const char* name() const { return "get current contended monitor"; }      
};

// VM operation to get stack trace at safepoint.
class VM_GetStackTrace : public VM_Operation {
private:
  JvmtiEnv *_env;
  JavaThread *_java_thread;
  jint _start_depth;
  jint _max_count;
  jvmtiFrameInfo *_frame_buffer;
  jint *_count_ptr;
  jvmtiError _result;

public:
  VM_GetStackTrace(JvmtiEnv *env, JavaThread *java_thread, 
                   jint start_depth, jint max_count,
                   jvmtiFrameInfo* frame_buffer, jint* count_ptr) {
    _env = env;
    _java_thread = java_thread;
    _start_depth = start_depth;
    _max_count = max_count;
    _frame_buffer = frame_buffer;
    _count_ptr = count_ptr;
  }
  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase *)_env)->get_stack_trace(_java_thread,
                                                      _start_depth, _max_count,
                                                      _frame_buffer, _count_ptr);
  }
  const char* name() const { return "get stack trace"; }          
};

// forward declaration
struct StackInfoNode;

// VM operation to get stack trace at safepoint.
class VM_GetMultipleStackTraces : public VM_Operation {
private:
  JvmtiEnv *_env;
  jint _max_frame_count;
  jvmtiStackInfo *_stack_info;
  jvmtiError _result;
  int _frame_count_total;
  struct StackInfoNode *_head;

  JvmtiEnvBase *env()                 { return (JvmtiEnvBase *)_env; }
  jint max_frame_count()              { return _max_frame_count; }
  struct StackInfoNode *head()        { return _head; }
  void set_head(StackInfoNode *head)  { _head = head; }

protected:
  void set_result(jvmtiError result)  { _result = result; }
  void fill_frames(jthread jt, JavaThread *thr, oop thread_oop);
  void allocate_and_fill_stacks(jint thread_count);

public:
  VM_GetMultipleStackTraces(JvmtiEnv *env, jint max_frame_count) {
    _env = env;
    _max_frame_count = max_frame_count;
    _frame_count_total = 0;
    _head = NULL;
    _result = JVMTI_ERROR_NONE;
  }
  jvmtiStackInfo *stack_info()       { return _stack_info; }
  jvmtiError result()                { return _result; }

  const char* name() const { return "abstract get multiple stack traces"; }          
};


// VM operation to get stack trace at safepoint.
class VM_GetAllStackTraces : public VM_GetMultipleStackTraces {
private:
  jint _final_thread_count;

public:
  VM_GetAllStackTraces(JvmtiEnv *env, jint max_frame_count) 
      : VM_GetMultipleStackTraces(env, max_frame_count) {
  }
  void doit();
  jint final_thread_count()       { return _final_thread_count; }

  const char* name() const        { return "get all stack traces"; }          
};

// VM operation to get stack trace at safepoint.
class VM_GetThreadListStackTraces : public VM_GetMultipleStackTraces {
private:
  jint _thread_count;
  const jthread* _thread_list;

public:
  VM_GetThreadListStackTraces(JvmtiEnv *env, jint thread_count, const jthread* thread_list, jint max_frame_count) 
      : VM_GetMultipleStackTraces(env, max_frame_count) {
    _thread_count = thread_count;
    _thread_list = thread_list;
  }
  void doit();

  const char* name() const        { return "get thread list stack traces"; }          
};


// VM operation to count stack frames at safepoint.
class VM_GetFrameCount : public VM_Operation {
private:
  JvmtiEnv *_env;
  JvmtiThreadState *_state;
  jint *_count_ptr;
  jvmtiError _result;

public:
  VM_GetFrameCount(JvmtiEnv *env, JvmtiThreadState *state, jint *count_ptr) {
    _env = env;
    _state = state;
    _count_ptr = count_ptr;
  }

  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase*)_env)->get_frame_count(_state, _count_ptr);
  }
  const char* name() const { return "get frame count"; }          
};

// VM operation to frame location at safepoint.
class VM_GetFrameLocation : public VM_Operation {
private:
  JvmtiEnv *_env;
  JavaThread* _java_thread;
  jint _depth;
  jmethodID* _method_ptr;
  jlocation* _location_ptr;
  jvmtiError _result;

public:
  VM_GetFrameLocation(JvmtiEnv *env, JavaThread* java_thread, jint depth, 
                      jmethodID* method_ptr, jlocation* location_ptr) {
    _env = env;
    _java_thread = java_thread;
    _depth = depth;
    _method_ptr = method_ptr;
    _location_ptr = location_ptr;
  }

  jvmtiError result() { return _result; }
  void doit() {
    _result = ((JvmtiEnvBase*)_env)->get_frame_location(_java_thread, _depth,
                                                        _method_ptr, _location_ptr);
  }
  const char* name() const { return "get frame location"; }          
};


// ResourceTracker
//
// ResourceTracker works a little like a ResourceMark. All allocates
// using the resource tracker are recorded. If an allocate using the
// resource tracker fails the destructor will free any resources
// that were allocated using the tracker. 
// The motive for this class is to avoid messy error recovery code
// in situations where multiple allocations are done in sequence. If
// the second or subsequent allocation fails it avoids any code to
// release memory allocated in the previous calls.
//
// Usage :-
//   ResourceTracker rt(env);
//   :
//   err = rt.allocate(1024, &ptr);

class ResourceTracker : public StackObj {
 private:
  JvmtiEnv* _env;
  GrowableArray<unsigned char*> *_allocations;	
  bool _failed;
 public:
  ResourceTracker(JvmtiEnv* env);
  ~ResourceTracker();
  jvmtiError allocate(jlong size, unsigned char** mem_ptr);
  unsigned char* allocate(jlong size);
  char* strdup(const char* str);
};

#endif   /* _JAVA_JVMTIENVBASE_H_ */
