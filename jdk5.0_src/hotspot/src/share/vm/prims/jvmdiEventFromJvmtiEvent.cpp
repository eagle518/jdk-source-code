#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdiEventFromJvmtiEvent.cpp	1.16 03/12/23 16:43:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


# include "incls/_precompiled.incl"
# include "incls/_jvmdiEventFromJvmtiEvent.cpp.incl"

static JVMDI_EventHook hook = NULL;

static jvmtiEventCallbacks jvmdi_callbacks;

static jframeID
current_frame(jthread thread) {
  oop thread_oop = JNIHandles::resolve_external_guard(thread);
  JavaThread* java_thread = java_lang_Thread::thread(thread_oop);
  JvmtiThreadState* state = JvmtiThreadState::state_for(java_thread);
  // Return the cached jframeID. This thread is running in native code
  // so we can not call depth_to_jframeID() to compute and get
  // jframeID which may required to hold JvmdiFrame mutex lock.
  return state->jvmdi_cached_frames()->event_jframeID();
}
 

// extract the jclass from the method id and convert the method id to
// OBSOLETE_METHOD_ID if need be
static jclass 
method_class(JNIEnv *jni_env, jmethodID mid, jmethodID *jvmdi_mid_p) {
  // some events can return NULL jmethodIDs (catch method of exception)
  if (mid == NULL) {
    *jvmdi_mid_p = NULL;   // which, of course, is the same as OBSOLETE_METHOD_ID but that 
    return NULL;           // is what the JVMDI spec wants, and the class differentiates
  }
  oop k_mirror = JvmtiEnv::jvmti_env_for_jvmdi()->get_jvmdi_method_and_class(mid, jvmdi_mid_p);
  jobject jc = JNIHandles::make_local(jni_env, k_mirror);
  return (jclass)jc;
}

extern "C" {

static void
JNICALL SingleStep(jvmtiEnv *jvmti_env,
           JNIEnv *jni_env,
	   jthread thread,
	   jmethodID method,
	   jlocation location) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_SINGLE_STEP;
  e.u.single_step.thread = thread;
  e.u.single_step.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.single_step.method = jvmdi_method_id;
  e.u.single_step.location = location;
  
  hook(jni_env, &e);
}

static void
JNICALL Breakpoint(jvmtiEnv *jvmti_env,
           JNIEnv *jni_env,
	   jthread thread,
	   jmethodID method,
	   jlocation location) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_BREAKPOINT;
  e.u.breakpoint.thread = thread;
  e.u.breakpoint.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.breakpoint.method = jvmdi_method_id;
  e.u.breakpoint.location = location;
  
  hook(jni_env, &e);
}

static void
JNICALL FieldAccess(jvmtiEnv *jvmti_env,
            JNIEnv *jni_env,
	    jthread thread,
	    jmethodID method,
	    jlocation location,
	    jclass field_klass,
	    jobject object,
	    jfieldID field) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_FIELD_ACCESS;
  e.u.field_access.thread = thread;
  e.u.field_access.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.field_access.method = jvmdi_method_id;
  e.u.field_access.location = location;
  e.u.field_access.field_clazz = field_klass;
  e.u.field_access.object = object;
  e.u.field_access.field = field;
  
  hook(jni_env, &e);

}

static void
JNICALL FieldModification(jvmtiEnv *jvmti_env,
                  JNIEnv *jni_env,
		  jthread thread,
		  jmethodID method,
		  jlocation location,
		  jclass field_klass,
		  jobject object,
		  jfieldID field,
		  char signature_type,
		  jvalue new_value) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_FIELD_MODIFICATION;
  e.u.field_modification.thread = thread;
  e.u.field_modification.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.field_modification.method = jvmdi_method_id;
  e.u.field_modification.location = location;
  e.u.field_modification.field_clazz = field_klass;
  e.u.field_modification.object = object;
  e.u.field_modification.field = field;
  e.u.field_modification.signature_type = signature_type;
  e.u.field_modification.new_value = new_value;
  
  hook(jni_env, &e);
}

static void
JNICALL FramePop(jvmtiEnv *jvmti_env,
         JNIEnv *jni_env,
         jthread thread,
         jmethodID method,
	 jboolean wasPoppedByException) {
  jmethodID jvmdi_method_id;
  if (!wasPoppedByException) {
    JVMDI_Event e;
    e.kind = JVMDI_EVENT_FRAME_POP;
    e.u.frame.thread = thread;
    e.u.frame.clazz = method_class(jni_env, method, &jvmdi_method_id);
    e.u.frame.method = jvmdi_method_id;
    e.u.frame.frame = current_frame(thread);

    hook(jni_env, &e);
  }
}

static void
JNICALL MethodEntry(jvmtiEnv *jvmti_env,
            JNIEnv *jni_env,
	    jthread thread,
	    jmethodID method) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_METHOD_ENTRY;
  e.u.frame.thread = thread;
  e.u.frame.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.frame.method = jvmdi_method_id;
  e.u.frame.frame = current_frame(thread);

  hook(jni_env, &e);
}

static void
JNICALL MethodExit(jvmtiEnv *jvmti_env,
           JNIEnv *jni_env,
	   jthread thread,
	   jmethodID method,
	   jboolean wasPoppedByException,
	   jvalue return_value) {
  if (!wasPoppedByException) {
    jmethodID jvmdi_method_id;
    JVMDI_Event e;
    e.kind = JVMDI_EVENT_METHOD_EXIT;
    e.u.frame.thread = thread;
    e.u.frame.clazz = method_class(jni_env, method, &jvmdi_method_id);
    e.u.frame.method = jvmdi_method_id;
    e.u.frame.frame = current_frame(thread);

    hook(jni_env, &e);
  }
}

static void
JNICALL Exception(jvmtiEnv *jvmti_env,
           JNIEnv *jni_env,
	  jthread thread,
	  jmethodID method,
	  jlocation location,
	  jobject exception,
	  jmethodID catch_method,
	  jlocation catch_location) {
  jmethodID jvmdi_method_id;
  jmethodID jvmdi_catch_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_EXCEPTION;
  e.u.exception.thread = thread;
  e.u.exception.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.exception.method = jvmdi_method_id;
  e.u.exception.location = location;
  e.u.exception.exception = exception;
  e.u.exception.catch_clazz = method_class(jni_env, catch_method, &jvmdi_catch_method_id);
  e.u.exception.catch_method = jvmdi_catch_method_id;
  e.u.exception.catch_location = catch_location;
  
  hook(jni_env, &e);
}

static void
JNICALL ExceptionCatch(jvmtiEnv *jvmti_env,
               JNIEnv *jni_env,
	       jthread thread,
	       jmethodID method,
	       jlocation location,
	       jobject exception) {
  jmethodID jvmdi_method_id;
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_EXCEPTION_CATCH;
  e.u.exception_catch.thread = thread;
  e.u.exception_catch.clazz = method_class(jni_env, method, &jvmdi_method_id);
  e.u.exception_catch.method = jvmdi_method_id;
  e.u.exception_catch.location = location;
  e.u.exception_catch.exception = exception;
  
  hook(jni_env, &e);
}

static void
JNICALL ThreadStart(jvmtiEnv *jvmti_env,
            JNIEnv *jni_env,
	    jthread thread) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_THREAD_START;
  e.u.thread_change.thread = thread;
  
  hook(jni_env, &e);
}

static void
JNICALL ThreadEnd(jvmtiEnv *jvmti_env,
          JNIEnv *jni_env,
	  jthread thread) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_THREAD_END;
  e.u.thread_change.thread = thread;
  
  hook(jni_env, &e);
}

static void
JNICALL ClassLoad(jvmtiEnv *jvmti_env,
          JNIEnv *jni_env,
	  jthread thread,
	  jclass klass) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_CLASS_LOAD;
  e.u.class_event.thread = thread;
  e.u.class_event.clazz = klass;
  
  hook(jni_env, &e);
}

static void
JNICALL ClassUnload(jvmtiEnv *jvmti_env,
            JNIEnv *jni_env,
	    jthread thread,
	    jclass klass, ...) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_CLASS_UNLOAD;
  e.u.class_event.thread = thread;
  e.u.class_event.clazz = klass;
  
  hook(jni_env, &e);
}

static void
JNICALL ClassPrepare(jvmtiEnv *jvmti_env,
             JNIEnv *jni_env,
	     jthread thread,
	     jclass klass) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_CLASS_PREPARE;
  e.u.class_event.thread = thread;
  e.u.class_event.clazz = klass;
  
  hook(jni_env, &e);
}

static void
JNICALL VMInit(jvmtiEnv *jvmti_env,
               JNIEnv *jni_env,
               jthread thread) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_VM_INIT;
  
  hook(jni_env, &e);
}

static void
JNICALL VMDeath(jvmtiEnv *jvmti_env,
        JNIEnv *jni_env) {
  JVMDI_Event e;
  e.kind = JVMDI_EVENT_VM_DEATH;
  
  hook(jni_env, &e);
}

} /* end extern "C" */;

jvmtiEventMode JvmdiEventFromJvmtiEvent::_jvmdi_class_unload_enabled_mode = JVMTI_DISABLE;

jvmtiEventCallbacks*
JvmdiEventFromJvmtiEvent::set_jvmdi_event_hook(JVMDI_EventHook new_hook) {
  hook = new_hook;

  // update the class unload callback
  set_class_unload_callback_for_jvmdi();

  if (new_hook == NULL) {
    return NULL;
  } else {
    jvmdi_callbacks.SingleStep = (jvmtiEventSingleStep)SingleStep;
    jvmdi_callbacks.Breakpoint = (jvmtiEventBreakpoint)Breakpoint;
    jvmdi_callbacks.FieldAccess = (jvmtiEventFieldAccess)FieldAccess;
    jvmdi_callbacks.FieldModification = (jvmtiEventFieldModification)FieldModification;
    jvmdi_callbacks.FramePop = (jvmtiEventFramePop)FramePop;
    jvmdi_callbacks.MethodEntry = (jvmtiEventMethodEntry)MethodEntry;
    jvmdi_callbacks.MethodExit = (jvmtiEventMethodExit)MethodExit;
    jvmdi_callbacks.Exception = (jvmtiEventException)Exception;
    jvmdi_callbacks.ExceptionCatch = (jvmtiEventExceptionCatch)ExceptionCatch;
    jvmdi_callbacks.ThreadStart = (jvmtiEventThreadStart)ThreadStart;
    jvmdi_callbacks.ThreadEnd = (jvmtiEventThreadEnd)ThreadEnd;
    jvmdi_callbacks.ClassLoad = (jvmtiEventClassLoad)ClassLoad;
    jvmdi_callbacks.ClassPrepare = (jvmtiEventClassPrepare)ClassPrepare;
    jvmdi_callbacks.VMInit = (jvmtiEventVMInit)VMInit;
    jvmdi_callbacks.VMDeath = (jvmtiEventVMDeath)VMDeath;
    return &jvmdi_callbacks;
  }
}

void 
JvmdiEventFromJvmtiEvent::set_jvmdi_class_unload_enabled_mode(jvmtiEventMode mode) {
  // save the enable/disable mode for later since the hook may change
  _jvmdi_class_unload_enabled_mode = mode;

  // update the class unload callback
  set_class_unload_callback_for_jvmdi();
}

// Convert a JVMDI class_unload event into an extension event.
// Unlike regular JVMDI/JVMTI events, extension events do not
// have a separate enabling/disable mechanism and are called if
// the callback is set.
// Set the JVMTI extension event callback to the JVMDI event 
// translation callback if the class unload event is enabled and the
// JVMDI event hook is set, otherwise clear it.
void
JvmdiEventFromJvmtiEvent::set_class_unload_callback_for_jvmdi() {
  jvmtiExtensionEvent unload_cb;
  if (_jvmdi_class_unload_enabled_mode == JVMTI_ENABLE && hook != NULL) {
    unload_cb = (jvmtiExtensionEvent)ClassUnload;
  } else {
    unload_cb = NULL;
  } 
  jvmtiError err = JvmtiExtensions::set_event_callback(JvmtiEnvBase::jvmti_env_for_jvmdi(), 
                                                       EXT_EVENT_CLASS_UNLOAD, unload_cb);							   
  guarantee(err == JVMTI_ERROR_NONE, "extension event not registered");
}
