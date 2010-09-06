#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmtiEnvBase.cpp	1.47 04/03/23 12:05:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */
# include "incls/_precompiled.incl"
# include "incls/_jvmtiEnvBase.cpp.incl"


///////////////////////////////////////////////////////////////
//
// JvmtiEnvBase
//

GrowableArray<JvmtiEnvBase*> *JvmtiEnvBase::_environments = new (ResourceObj::C_HEAP) GrowableArray<JvmtiEnvBase*>(2,true);

JvmtiEnv* JvmtiEnvBase::_jvmti_env_for_jvmdi = NULL;

bool JvmtiEnvBase::_globally_initialized = false;

jvmtiPhase JvmtiEnvBase::_phase = JVMTI_PHASE_PRIMORDIAL;

extern jvmtiInterface_1_ jvmti_Interface;
extern jvmtiInterface_1_ jvmtiTrace_Interface;


// perform initializations that must occur before any JVMTI environments
// are released but which should only be initialized once (no matter
// how many environments are created).
void
JvmtiEnvBase::globally_initialize() {
  assert(Threads::number_of_threads() == 0 || JvmtiThreadState_lock->is_locked(), "sanity check");
  assert(_globally_initialized == false, "bad call");

  JvmtiManageCapabilities::initialize();

  // register extension functions and events
  JvmtiExtensions::register_extensions();

#ifdef JVMTI_TRACE
  JvmtiTrace::initialize();
#endif

  _globally_initialized = true;
}


void
JvmtiEnvBase::initialize() {
  assert(Threads::number_of_threads() == 0 || JvmtiThreadState_lock->is_locked(), "sanity check");

  // add this environment
  _environments->append((JvmtiEnv*)this);
  _index = _environments->length() - 1;

  if (_globally_initialized == false) {
    globally_initialize();
  }
}


JvmtiEnvBase::JvmtiEnvBase(bool jvmdi) : _env_event_enable() {
  _jvmdi = jvmdi;
  _ahook = NULL;
  _dhook = NULL;
  _env_local_storage = NULL;
  _tag_map = NULL;

  // all callbacks initially NULL
  memset(&_event_callbacks,0,sizeof(jvmtiEventCallbacks));

  // all capabilities initially off
  memset(&_current_capabilities, 0, sizeof(_current_capabilities));

  JvmtiEventController::env_initialize((JvmtiEnv*)this);

  if (_jvmdi) {
    _magic = JVMDI_MAGIC;
  } else {
    _magic = JVMTI_MAGIC;
#ifdef JVMTI_TRACE
    _jvmti_external.functions = strlen(TraceJVMTI)? &jvmtiTrace_Interface : &jvmti_Interface;
#else
    _jvmti_external.functions = &jvmti_Interface;
#endif
  }
}


void
JvmtiEnvBase::dispose() {
  assert(Threads::number_of_threads() == 0 || JvmtiThreadState_lock->is_locked(), "sanity check");

#ifdef JVMTI_TRACE
  JvmtiTrace::shutdown();
#endif

  // remove this environment, then reset the indicies for remaining environments
  _environments->remove(this);
  for (int i = 0; i < _environments->length(); ++i) {
    _environments->at(i)->_index = i;
  }
}


JvmtiEnvBase::~JvmtiEnvBase(){
  _magic = BAD_MAGIC;
  JvmtiEventController::env_dispose(this);
  // Relinquish all capabilities.
  jvmtiCapabilities *caps = get_capabilities();
  JvmtiManageCapabilities::relinquish_capabilities(caps, caps, caps);
  // Destroy the tag map
  if (_tag_map != NULL) {
    delete _tag_map;
  }
}


void
JvmtiEnvBase::set_event_callbacks(const jvmtiEventCallbacks* callbacks,
                                               jint size_of_callbacks) {
  assert(Threads::number_of_threads() == 0 || JvmtiThreadState_lock->is_locked(), "sanity check");

  size_t byte_cnt = sizeof(jvmtiEventCallbacks);

  // clear in either case to be sure we got any gap between sizes
  memset(&_event_callbacks, 0, byte_cnt);
  if (callbacks != NULL) {
    if (size_of_callbacks < (jint)byte_cnt) {
      byte_cnt = size_of_callbacks;
    }
    memcpy(&_event_callbacks, callbacks, byte_cnt);
  }
}


// Called from JVMTI entry points which perform stack walking. If the
// associated JavaThread is the current thread, then wait_for_suspend
// is not used. Otherwise, it determines if we should wait for the
// "other" thread to complete external suspension. (NOTE: in future
// releases the suspension mechanism should be reimplemented so this
// is not necessary.)
//
bool 
JvmtiEnvBase::is_thread_fully_suspended(JavaThread* thr, bool wait_for_suspend, uint32_t *bits) {
  // "other" threads require special handling
  if (thr != JavaThread::current()) {
    if (wait_for_suspend) {
      // We are allowed to wait for the external suspend to complete
      // so give the other thread a chance to get suspended.
      if (!thr->wait_for_ext_suspend_completion(SUSPENDRETRYCOUNT, bits)) {
        // didn't make it so let the caller know
        return false;
      }
    }
    // We aren't allowed to wait for the external suspend to complete
    // so if the other thread isn't externally suspended we need to
    // let the caller know.
    else if (!thr->is_ext_suspend_completed_with_lock(bits)) {
      return false;
    }
  }

  return true;
}


jvmtiError
JvmtiEnvBase::jvmdi_allocate(jlong size, unsigned char** mem_ptr) {
  if (size < 0) {
    return JVMTI_ERROR_ILLEGAL_ARGUMENT;
  }
  if (_ahook == NULL) {
    *mem_ptr = (unsigned char *)os::malloc((size_t)size); 
    if (*mem_ptr == NULL) {
      return JVMTI_ERROR_OUT_OF_MEMORY;
    }
    return JVMTI_ERROR_NONE;
  } else {
    if (Threads::number_of_threads() > 0) {      
      JavaThread* current_thread = JavaThread::current();
      ThreadToNativeFromVM transition(current_thread);

      // This HandleMark is essential, without it, if the alloc function calls a JNI or other
      // JVMTI function (for example, RawMonitorEnter), as it exits, its HandleMarkCleaner 
      // destructor will run and destroy any handles the requestor of the alloc was holding.
      HandleMark hm(current_thread);

      return (jvmtiError)_ahook(size, (jbyte**)mem_ptr);
    } else {
      return (jvmtiError)_ahook(size, (jbyte**)mem_ptr);
    }
  }
} /* end allocate */


jvmtiError
JvmtiEnvBase::jvmdi_deallocate(unsigned char* mem) {
  if (_dhook == NULL) {
    if (mem != NULL) {
      os::free(mem);
    } else {
      return JVMTI_ERROR_NULL_POINTER;
    }
    return JVMTI_ERROR_NONE;
  } else {
    if (Threads::number_of_threads() > 0) {
      JavaThread* current_thread = JavaThread::current();
      ThreadToNativeFromVM transition(current_thread);
      HandleMark hm(current_thread);  // See comment on Allocate HandleMark
      return (jvmtiError)_dhook((jbyte*)mem);
    } else {
      return (jvmtiError)_dhook((jbyte*)mem);
    }
  }
} /* end deallocate */


// In the fullness of time, all users of the method should instead
// directly use allocate, besides being cleaner and faster, this will
// mean much better out of memory handling
unsigned char *
JvmtiEnvBase::jvmtiMalloc(jlong size) {
  unsigned char* mem;
  jvmtiError result = allocate(size, &mem);
  assert(result == JVMTI_ERROR_NONE, "Allocate failed");
  return mem;
}

  
//
// Threads
//

jobject *
JvmtiEnvBase::new_jobjectArray(int length, Handle *handles) {
  if (length == 0) {
    return NULL;
  }

  jobject *objArray = (jobject *) jvmtiMalloc(sizeof(jobject) * length);
  NULL_CHECK(objArray, NULL);
  
  for (int i=0; i<length; i++) {
    objArray[i] = jni_reference(handles[i]);
  }
  return objArray;
}

jthread *
JvmtiEnvBase::new_jthreadArray(int length, Handle *handles) {
  return (jthread *) new_jobjectArray(length,handles);
}

jthreadGroup *
JvmtiEnvBase::new_jthreadGroupArray(int length, Handle *handles) {
  return (jthreadGroup *) new_jobjectArray(length,handles);
}


JavaThread *
JvmtiEnvBase::get_JavaThread(jthread jni_thread) {
  oop t = JNIHandles::resolve_external_guard(jni_thread);
  if (t == NULL || !t->is_a(SystemDictionary::thread_klass())) {
    return NULL;
  }
  // The following returns NULL if the thread has not yet run or is in
  // process of exiting
  return java_lang_Thread::thread(t);
}


// update the access_flags for the field in the klass
void
JvmtiEnvBase::update_klass_field_access_flag(fieldDescriptor *fd) {
  instanceKlass* ik = instanceKlass::cast(fd->field_holder());
  typeArrayOop fields = ik->fields();
  fields->ushort_at_put(fd->index(), (jushort)fd->access_flags().as_short());
}


// return the vframe on the specified thread and depth, NULL if no such frame
vframe*
JvmtiEnvBase::vframeFor(JavaThread* java_thread, jint depth) {
  if (!java_thread->has_last_Java_frame()) {
    return NULL;
  }
  RegisterMap reg_map(java_thread);
  vframe *vf = java_thread->last_java_vframe(&reg_map);
  int d = 0;
  while ((vf != NULL) && (d < depth)) {
    vf = vf->java_sender();
    d++;
  }
  return vf;
}


//
// utilities: JNI objects
//

oop 
JvmtiEnvBase::get_jvmdi_method_and_class(jmethodID mid, jmethodID *mid_p) {
  methodOop method_oop = jniIdSupport::to_method_oop(mid);
  // we can only test a refetched methodOop since obsoleteness may be indirect -
  // that is, current method version is A2, we have A0, but A1 is non-EMCP - 
  // method id for A0 will be method id of A1
  if (method_oop->is_old_version()) {
    // For old non-EMCP versions of methods of evolved classes we return obsolete
    assert(method_oop->is_non_emcp_with_new_version(), "must not be obsolete");
    *mid_p = OBSOLETE_METHOD_ID;
  } else {
    *mid_p =  mid;
  }
  klassOop k = method_oop->method_holder();
  return Klass::cast(k)->java_mirror();
}


jclass
JvmtiEnvBase::get_jni_class_non_null(klassOop k) {
  assert(k != NULL, "k != NULL");
  return (jclass)jni_reference(Klass::cast(k)->java_mirror());
}

// 
// Field Information 
// 

bool 
JvmtiEnvBase::get_field_descriptor(klassOop k, jfieldID field, fieldDescriptor* fd) { 
  if (!jfieldIDWorkaround::is_valid_jfieldID(k, field)) { 
    return false; 
  } 
  bool found = false; 
  if (jfieldIDWorkaround::is_static_jfieldID(field)) { 
    JNIid* id = jfieldIDWorkaround::from_static_jfieldID(field); 
    int offset = id->offset(); 
    klassOop holder = id->holder(); 
    found = instanceKlass::cast(holder)->find_local_field_from_offset(offset, true, fd); 
  } else { 
    // Non-static field. The fieldID is really the offset of the field within the object. 
    int offset = jfieldIDWorkaround::from_instance_jfieldID(k, field); 
    found = instanceKlass::cast(k)->find_field_from_offset(offset, false, fd); 
  } 
  return found; 
} 

//
// Object Monitor Information
//

//
// Count the number of objects for a lightweight monitor. The hobj
// parameter is object that owns the monitor so this routine will
// count the number of times the same object was locked by frames
// in java_thread.
//
jint
JvmtiEnvBase::count_locked_objects(JavaThread *java_thread, Handle hobj) {
  jint ret = 0;
  if (!java_thread->has_last_Java_frame()) {
    return ret;  // no Java frames so no monitors
  }

  ResourceMark rm;
  HandleMark   hm;
  RegisterMap  reg_map(java_thread);

  for(javaVFrame *jvf=java_thread->last_java_vframe(&reg_map); jvf != NULL;
                                                 jvf = jvf->java_sender()) {
    GrowableArray<MonitorInfo*>* mons = jvf->monitors();
    if (!mons->is_empty()) {
      for (int i = 0; i < mons->length(); i++) {
        MonitorInfo *mi = mons->at(i);

        // see if owner of the monitor is our object
        if (mi->owner() != NULL && mi->owner() == hobj()) {
          ret++;
        }
      }
    }
  }
  return ret;
}



jvmtiError
JvmtiEnvBase::get_current_contended_monitor(JavaThread *java_thread, jobject *monitor_ptr) {
#ifdef ASSERT
  uint32_t debug_bits = 0;
#endif
  assert((SafepointSynchronize::is_at_safepoint() ||
          is_thread_fully_suspended(java_thread, false, &debug_bits)),
         "at safepoint or target thread is suspended");    
  oop obj = NULL;
  ObjectMonitor *mon = java_thread->current_waiting_monitor();
  if (mon == NULL) {
    // thread is not doing an Object.wait() call
    mon = java_thread->current_pending_monitor();
    if (mon != NULL) { 
      // The thread is trying to enter() or raw_enter() an ObjectMonitor.
      obj = (oop)mon->object();
      // If obj == NULL, then ObjectMonitor is raw which doesn't count
      // as contended for this API
    }
    // implied else: no contended ObjectMonitor
  } else {
    // thread is doing an Object.wait() call
    obj = (oop)mon->object();
    assert(obj != NULL, "Object.wait() should have an object");
  }

  if (obj == NULL) {
    *monitor_ptr = NULL;
  } else {
    HandleMark hm;
    Handle     hobj(obj);
    *monitor_ptr = jni_reference(hobj);
  }
  return JVMTI_ERROR_NONE;
} 

jvmtiError
JvmtiEnvBase::get_owned_monitor_info(JavaThread *java_thread,
                                     jint* owned_monitor_count_ptr,
                                     jobject** owned_monitors_ptr) {
#ifdef ASSERT
  uint32_t debug_bits = 0;
#endif
  assert((SafepointSynchronize::is_at_safepoint() ||
          is_thread_fully_suspended(java_thread, false, &debug_bits)),
         "at safepoint or target thread is suspended");
  jvmtiError err = JVMTI_ERROR_NONE;
  Thread* current_thread = Thread::current();
  ResourceMark rm(current_thread);
  // growable array of locked object handles on the C-heap
  GrowableArray<jobject> *locked_object_list = new GrowableArray<jobject>(1, true);

  if (java_thread->has_last_Java_frame()) {
    ResourceMark rm;
    HandleMark   hm;
    RegisterMap  reg_map(java_thread);

    int depth = 0;
    for (javaVFrame *jvf = java_thread->last_java_vframe(&reg_map); jvf != NULL;
         jvf = jvf->java_sender()) {
      
      if (depth++ < MaxJavaStackTraceDepth) {  // check for stack too deep
        // add locked objects for this frame into list
        get_locked_objects_in_frame(java_thread, jvf, locked_object_list);  
      }
    }
  }

  jint owned_monitor_count = locked_object_list->length();

  if ((err = allocate(owned_monitor_count * sizeof(jobject *), 
                      (unsigned char**)owned_monitors_ptr)) != JVMTI_ERROR_NONE) {
    return err;
  }

  // copy into the returned array
  for (int i = 0; i < owned_monitor_count; i++) {
    (*owned_monitors_ptr)[i] = (jobject)locked_object_list->at(i);
  }
  *owned_monitor_count_ptr = owned_monitor_count;
  
  return err;
}

// Save JNI handles (local for JVMTI, global for JVMDI) for any objects that
// this frame owns.
void 
JvmtiEnvBase::get_locked_objects_in_frame(JavaThread* java_thread, javaVFrame *jvf,
                                 GrowableArray<jobject>* locked_object_list) {
  ResourceMark rm;

  GrowableArray<MonitorInfo*>* mons = jvf->monitors();
  if (mons->is_empty()) {
    return;  // this javaVFrame holds no monitors
  }

  HandleMark hm;
  oop wait_obj = NULL;
  {
    // save object of current wait() call (if any) for later comparison
    ObjectMonitor *mon = java_thread->current_waiting_monitor();
    if (mon != NULL) {
      wait_obj = (oop)mon->object();
    }
  }
  oop pending_obj = NULL;
  {
    // save object of current enter() call (if any) for later comparison
    ObjectMonitor *mon = java_thread->current_pending_monitor();
    if (mon != NULL) {
      pending_obj = (oop)mon->object();
    }
  }

  for (int i = 0; i < mons->length(); i++) {
    MonitorInfo *mi = mons->at(i);

    oop obj = mi->owner();
    if (obj == NULL) {
      // this monitor doesn't have an owning object so skip it
      continue;
    }

    if (wait_obj == obj) {
      // the thread is waiting on this monitor so it isn't really owned
      continue;
    }

    if (pending_obj == obj) {
      // the thread is pending on this monitor so it isn't really owned
      continue;
    }

    if (locked_object_list->length() > 0) {
      // Our list has at least one object on it so we have to check
      // for recursive object locking
      bool found = false;
      for (int j = 0; j < locked_object_list->length(); j++) {
        jobject jobj = locked_object_list->at(j);
        oop check = JNIHandles::resolve(jobj);
        if (check == obj) {
          found = true;  // we found the object
          break;
        }
      }

      if (found) {
        // already have this object so don't include it
        continue;
      }
    }

    // add the owning object to our list
    Handle hobj(obj);
    jobject jobj = jni_reference(hobj);
    locked_object_list->append(jobj);
  }
}

jvmtiError
JvmtiEnvBase::get_stack_trace(JavaThread *java_thread, 
                              jint start_depth, jint max_count,
                              jvmtiFrameInfo* frame_buffer, jint* count_ptr) {
#ifdef ASSERT
  uint32_t debug_bits = 0;
#endif
  assert((SafepointSynchronize::is_at_safepoint() ||
          is_thread_fully_suspended(java_thread, false, &debug_bits)),
         "at safepoint or target thread is suspended");
  int count = 0;
  if (java_thread->has_last_Java_frame()) {
    RegisterMap reg_map(java_thread);
    Thread* current_thread = Thread::current(); 
    ResourceMark rm(current_thread);
    javaVFrame *jvf = java_thread->last_java_vframe(&reg_map);
    HandleMark hm(current_thread);
    if (start_depth != 0) {
      if (start_depth > 0) {
	for (int j = 0; j < start_depth && jvf != NULL; j++) {
	  jvf = jvf->java_sender();
	}
	if (jvf == NULL) {
	  // start_depth is deeper than the stack depth
	  return JVMTI_ERROR_ILLEGAL_ARGUMENT;
	}
      } else { // start_depth < 0
	// we are referencing the starting depth based on the oldest
	// part of the stack.
	// optimize to limit the number of times that java_sender() is called
	javaVFrame *jvf_cursor = jvf;
	javaVFrame *jvf_prev = NULL;
	javaVFrame *jvf_prev_prev;
	int j = 0;
	while (jvf_cursor != NULL) {
	  jvf_prev_prev = jvf_prev;
	  jvf_prev = jvf_cursor;
	  for (j = 0; j > start_depth && jvf_cursor != NULL; j--) {
	    jvf_cursor = jvf_cursor->java_sender();
	  }
	}
	if (j == start_depth) {
	  // previous pointer is exactly where we want to start
	  jvf = jvf_prev;
	} else {
	  // we need to back up further to get to the right place
	  if (jvf_prev_prev == NULL) {
	    // the -start_depth is greater than the stack depth
	    return JVMTI_ERROR_ILLEGAL_ARGUMENT;
	  }
	  // j now is the number of frames on the stack starting with
	  // jvf_prev, we start from jvf_prev_prev and move older on
	  // the stack that many, the result is -start_depth frames
	  // remaining.
	  jvf = jvf_prev_prev;
	  for (; j < 0; j++) {
	    jvf = jvf->java_sender();
	  }
	}	  
      }
    }
    for (; count < max_count && jvf != NULL; count++) {
      frame_buffer[count].method = jvf->method()->jmethod_id();
      frame_buffer[count].location = (jvf->method()->is_native() ? -1 : jvf->bci());
      jvf = jvf->java_sender();
    }
  } else {
    if (start_depth != 0) {
      // no frames and there is a starting depth
      return JVMTI_ERROR_ILLEGAL_ARGUMENT;
    }
  }
  *count_ptr = count;
  return JVMTI_ERROR_NONE;
}

jvmtiError
JvmtiEnvBase::get_frame_count(JvmtiThreadState *state, jint *count_ptr) {
  assert((state != NULL),
         "JavaThread should create JvmtiThreadState before calling this method");
  *count_ptr = state->count_frames();
  return JVMTI_ERROR_NONE;
}

jvmtiError
JvmtiEnvBase::get_frame_location(JavaThread *java_thread, jint depth, 
                                 jmethodID* method_ptr, jlocation* location_ptr) {
#ifdef ASSERT
  uint32_t debug_bits = 0;
#endif
  assert((SafepointSynchronize::is_at_safepoint() ||
          is_thread_fully_suspended(java_thread, false, &debug_bits)),
         "at safepoint or target thread is suspended");
  Thread* current_thread = Thread::current(); 
  ResourceMark rm(current_thread);

  vframe *vf = vframeFor(java_thread, depth);
  if (vf == NULL) {
    return JVMTI_ERROR_NO_MORE_FRAMES;
  }

  // vframeFor should return a java frame. If it doesn't
  // it means we've got an internal error and we return the
  // error in product mode. In debug mode we will instead
  // attempt to cast the vframe to a javaVFrame and will
  // cause an assertion/crash to allow further diagnosis.
#ifdef PRODUCT
  if (!vf->is_java_frame()) {
    return JVMTI_ERROR_INTERNAL;
  } 
#endif

  HandleMark hm(current_thread);
  javaVFrame *jvf = javaVFrame::cast(vf);
  methodOop method = jvf->method();
  if (method->is_native()) {
    *location_ptr = -1;
  } else {
    *location_ptr = jvf->bci();
  }
  *method_ptr = method->jmethod_id();

  return JVMTI_ERROR_NONE;
}


jvmtiError
JvmtiEnvBase::get_object_monitor_usage(jobject object, jvmtiMonitorUsage* info_ptr) {
  HandleMark hm;
  Handle hobj;

  bool at_safepoint = SafepointSynchronize::is_at_safepoint();

  // Check arguments
  {
    oop mirror = JNIHandles::resolve_external_guard(object);
    NULL_CHECK(mirror, JVMTI_ERROR_INVALID_OBJECT);
    NULL_CHECK(info_ptr, JVMTI_ERROR_NULL_POINTER);

    hobj = Handle(mirror);
  }

  JavaThread *owning_thread = NULL;
  ObjectMonitor *mon = NULL;
  jvmtiMonitorUsage ret = {
      NULL, 0, 0, NULL, 0, NULL
  };
  
  uint32_t debug_bits = 0;
  // first derive the object's owner and entry_count (if any)
  {
    address owner = NULL;
    {
      markOop mark = hobj()->mark();

      if (!mark->has_monitor()) {
        // this object has a lightweight monitor

        if (mark->has_locker()) {
          owner = (address)mark->locker(); // save the address of the Lock word
        }
        // implied else: no owner
      } else {
        // this object has a heavyweight monitor
        mon = mark->monitor();

        // The owner field of a heavyweight monitor may be NULL for no
        // owner, a JavaThread * or it may still be the address of the
        // Lock word in a JavaThread's stack. A monitor can be inflated
        // by a non-owning JavaThread, but only the owning JavaThread
        // can change the owner field from the Lock word to the
        // JavaThread * and it may not have done that yet.
        owner = (address)mon->owner();
      }
    }

    if (owner != NULL) {
      // This monitor is owned so we have to find the owning JavaThread.
      // Since owning_thread_from_monitor_owner() grabs a lock, GC can
      // move our object at this point. However, our owner value is safe
      // since it is either the Lock word on a stack or a JavaThread *.
      owning_thread = Threads::owning_thread_from_monitor_owner(owner, !at_safepoint);
      assert(owning_thread != NULL, "sanity check");
      if (owning_thread != NULL) {  // robustness
        // The monitor's owner either has to be the current thread, at safepoint 
        // or it has to be suspended. Any of these conditions will prevent both
        // contending and waiting threads from modifying the state of
        // the monitor.
        if (!at_safepoint && !JvmtiEnv::is_thread_fully_suspended(owning_thread, true, &debug_bits)) {
          return JVMTI_ERROR_THREAD_NOT_SUSPENDED;
        }
        HandleMark hm;
        Handle     th(owning_thread->threadObj());
        ret.owner = (jthread)jni_reference(th);
      }
      // implied else: no owner
    }

    if (owning_thread != NULL) {  // monitor is owned
      if ((address)owning_thread == owner) {
        // the owner field is the JavaThread *
        assert(mon != NULL,
          "must have heavyweight monitor with JavaThread * owner");
        ret.entry_count = mon->recursions() + 1;
      } else {
        // The owner field is the Lock word on the JavaThread's stack
        // so the recursions field is not valid. We have to count the
        // number of recursive monitor entries the hard way. We pass
        // a handle to survive any GCs along the way.
        ResourceMark rm;
        ret.entry_count = count_locked_objects(owning_thread, hobj);
      }
    }
    // implied else: entry_count == 0
  }

  int nWant,nWait;
  if (mon != NULL) {
    // this object has a heavyweight monitor
    nWant = mon->contentions(); // # of threads contending for monitor
    nWait = mon->waiters();     // # of threads in Object.wait()
    ret.waiter_count = nWant + nWait;
    ret.notify_waiter_count = nWait;
  } else {
    // this object has a lightweight monitor
    ret.waiter_count = 0;
    ret.notify_waiter_count = 0;
  }

  // Allocate memory for heavyweight and lightweight monitor.
  jvmtiError err;
  err = allocate(ret.waiter_count * sizeof(jthread *), (unsigned char**)&ret.waiters);
  if (err != JVMTI_ERROR_NONE) {
    return err;
  }
  err = allocate(ret.notify_waiter_count * sizeof(jthread *), 
                 (unsigned char**)&ret.notify_waiters);
  if (err != JVMTI_ERROR_NONE) {
    deallocate((unsigned char*)ret.waiters);
    return err;
  }
 
  // now derive the rest of the fields
  if (mon != NULL) {
    // this object has a heavyweight monitor

    // Number of waiters may actually be less than the waiter count.
    // So NULL out memory so that unused memory will be NULL.
    memset(ret.waiters, 0, ret.waiter_count * sizeof(jthread *));
    memset(ret.notify_waiters, 0, ret.notify_waiter_count * sizeof(jthread *));

    if (ret.waiter_count > 0) {
      // we have contending and/or waiting threads
      HandleMark hm;
      if (nWant > 0) {
        // we have contending threads
        ResourceMark rm;
        // get_pending_threads returns only java thread so we do not need to
        // check for  non java threads.          
        GrowableArray<JavaThread*>* wantList = Threads::get_pending_threads(
          nWant, (address)mon, !at_safepoint);
        if (wantList->length() < nWant) {
          // robustness: the pending list has gotten smaller
          nWant = wantList->length();
        }
        for (int i = 0; i < nWant; i++) {
          JavaThread *pending_thread = wantList->at(i);
          // If the monitor has no owner, then a non-suspended contending
          // thread could potentially change the state of the monitor by
          // entering it. The JVM/TI spec doesn't allow this.
          if (owning_thread == NULL && !at_safepoint &
              !JvmtiEnv::is_thread_fully_suspended(pending_thread, true, &debug_bits)) {
            if (ret.owner != NULL) {
              destroy_jni_reference(ret.owner);
            }
            for (int j = 0; j < i; j++) {
              destroy_jni_reference(ret.waiters[j]);
            }
            deallocate((unsigned char*)ret.waiters);
            deallocate((unsigned char*)ret.notify_waiters);
            return JVMTI_ERROR_THREAD_NOT_SUSPENDED;
          }
          Handle th(pending_thread->threadObj());
          ret.waiters[i] = (jthread)jni_reference(th);
        }
      }
      if (nWait > 0) {
        // we have threads in Object.wait()
        int offset = nWant;  // add after any contending threads
        ObjectWaiter *waiter = mon->first_waiter();
        for (int i = 0, j = 0; i < nWait; i++) {
          if (waiter == NULL) {
            // robustness: the waiting list has gotten smaller
            nWait = j;
            break;
          }
          Thread *t = mon->thread_of_waiter(waiter);
          if (t != NULL && t->is_Java_thread()) {
            JavaThread *wjava_thread = (JavaThread *)t;
            // If the thread was found on the ObjectWaiter list, then
            // it has not been notified. This thread can't change the
            // state of the monitor so it doesn't need to be suspended.
            Handle th(wjava_thread->threadObj());
            ret.waiters[offset + j] = (jthread)jni_reference(th);
            ret.notify_waiters[j++] = (jthread)jni_reference(th);
          }
          waiter = mon->next_waiter(waiter);
        }
      }
    }

    // Adjust count. nWant and nWait count values may be less than original.
    ret.waiter_count = nWant + nWait;
    ret.notify_waiter_count = nWait;
  } else {
    // this object has a lightweight monitor and we have nothing more
    // to do here because the defaults are just fine.
  }

  // we don't update return parameter unless everything worked
  if (is_jvmdi()) {
    // the JVMDI structure has less fields, don't smash surrounding data
    *((JVMDI_monitor_info*)info_ptr) = *((JVMDI_monitor_info*)&ret);
  } else {
    *info_ptr = ret;
  }

  return JVMTI_ERROR_NONE;
}

ResourceTracker::ResourceTracker(JvmtiEnv* env) {
  _env = env;
  _allocations = new (ResourceObj::C_HEAP) GrowableArray<unsigned char*>(20, true);
  _failed = false;
}
ResourceTracker::~ResourceTracker() {
  if (_failed) {
    for (int i=0; i<_allocations->length(); i++) {
      _env->deallocate(_allocations->at(i));	
    }
  }
  _allocations->clear_and_deallocate();
  delete _allocations;
}

jvmtiError ResourceTracker::allocate(jlong size, unsigned char** mem_ptr) {
  unsigned char *ptr;
  jvmtiError err = _env->allocate(size, &ptr);    
  if (err == JVMTI_ERROR_NONE) {
    _allocations->append(ptr);
    *mem_ptr = ptr;
  } else {
    *mem_ptr = NULL;
    _failed = true;
  }
  return err;
 }

unsigned char* ResourceTracker::allocate(jlong size) {    
  unsigned char* ptr;
  allocate(size, &ptr);
  return ptr;
}

char* ResourceTracker::strdup(const char* str) {
  char *dup_str = (char*)allocate(strlen(str)+1);
  if (dup_str != NULL) {
    strcpy(dup_str, str);
  }
  return dup_str;
}

struct StackInfoNode {
  struct StackInfoNode *next;
  jvmtiStackInfo info;
};

// Create a jvmtiStackInfo inside a linked list node and create a
// buffer for the frame information, both allocated as resource objects.
// Fill in both the jvmtiStackInfo and the jvmtiFrameInfo.
// Note that either or both of thr and thread_oop
// may be null if the thread is new or has exited.
void
VM_GetMultipleStackTraces::fill_frames(jthread jt, JavaThread *thr, oop thread_oop) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");   
 
  jint state = 0;
  struct StackInfoNode *node = NEW_RESOURCE_OBJ(struct StackInfoNode);
  jvmtiStackInfo *infop = &(node->info);
  node->next = head();
  set_head(node);
  infop->frame_count = 0;
  infop->thread = jt;
  
  if (thread_oop != NULL) {
    // get most state bits
    state = (jint)java_lang_Thread::get_thread_status(thread_oop);
  }
  
  if (thr != NULL) {    // add more state bits if there is a JavaThead to query
    // same as is_being_ext_suspended() but without locking
    if (thr->is_ext_suspended() || thr->is_external_suspend()) {
      state |= JVMTI_THREAD_STATE_SUSPENDED;
    }
    JavaThreadState jts = thr->thread_state();
    if (jts == _thread_in_native) {
      state |= JVMTI_THREAD_STATE_IN_NATIVE;
    }
    OSThread* osThread = thr->osthread(); 
    if (osThread != NULL && osThread->interrupted()) {
      state |= JVMTI_THREAD_STATE_INTERRUPTED;
    }
  }
  infop->state = state;
  
  if (thr != NULL || (state & JVMTI_THREAD_STATE_ALIVE) != 0) {
    infop->frame_buffer = NEW_RESOURCE_ARRAY(jvmtiFrameInfo, max_frame_count());
    env()->get_stack_trace(thr, 0, max_frame_count(),
                           infop->frame_buffer, &(infop->frame_count));
  } else {
    infop->frame_buffer = NULL;
    infop->frame_count = 0;
  }
  _frame_count_total += infop->frame_count;
}

// Based on the stack information in the linked list, allocate memory
// block to return and fill it from the info in the linked list.
void
VM_GetMultipleStackTraces::allocate_and_fill_stacks(jint thread_count) {
  // do I need to worry about alignment issues?
  jlong alloc_size =  thread_count       * sizeof(jvmtiStackInfo) 
                    + _frame_count_total * sizeof(jvmtiFrameInfo);
  env()->allocate(alloc_size, (unsigned char **)&_stack_info);

  // pointers to move through the newly allocated space as it is filled in
  jvmtiStackInfo *si = _stack_info + thread_count;      // bottom of stack info
  jvmtiFrameInfo *fi = (jvmtiFrameInfo *)si;            // is the top of frame info

  // copy information in resource area into allocated buffer
  // insert stack info backwards since linked list is backwards
  // insert frame info forwards
  // walk the StackInfoNodes
  for (struct StackInfoNode *sin = head(); sin != NULL; sin = sin->next) {
    jint frame_count = sin->info.frame_count;
    size_t frames_size = frame_count * sizeof(jvmtiFrameInfo);
    --si;
    memcpy(si, &(sin->info), sizeof(jvmtiStackInfo));
    if (frames_size == 0) {
      si->frame_buffer = NULL;
    } else {
      memcpy(fi, sin->info.frame_buffer, frames_size);
      si->frame_buffer = fi;  // point to the new allocated copy of the frames
      fi += frame_count;
    }
  }
  assert(si == _stack_info, "the last copied stack info must be the first record");
  assert((unsigned char *)fi == ((unsigned char *)_stack_info) + alloc_size, 
         "the last copied frame info must be the last record");
}


void
VM_GetThreadListStackTraces::doit() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");   

  ResourceMark rm;
  for (int i = 0; i < _thread_count; ++i) {
    jthread jt = _thread_list[i];
    oop thread_oop = JNIHandles::resolve_external_guard(jt);
    if (thread_oop == NULL || !thread_oop->is_a(SystemDictionary::thread_klass())) {
      set_result(JVMTI_ERROR_INVALID_THREAD);
      return;
    }
    fill_frames(jt, java_lang_Thread::thread(thread_oop), thread_oop);
  }
  allocate_and_fill_stacks(_thread_count);
}

void
VM_GetAllStackTraces::doit() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");   

  ResourceMark rm;
  _final_thread_count = 0;
  for (JavaThread *jt = Threads::first(); jt != NULL; jt = jt->next()) {
    oop thread_oop = jt->threadObj();
    if (thread_oop != NULL && 
        !jt->is_exiting() &&
        java_lang_Thread::is_alive(thread_oop) &&
        !jt->is_hidden_from_external_view()) {
      ++_final_thread_count;
      fill_frames((jthread)JNIHandles::make_local(thread_oop), jt, thread_oop);
    }
  }
  allocate_and_fill_stacks(_final_thread_count);
}

