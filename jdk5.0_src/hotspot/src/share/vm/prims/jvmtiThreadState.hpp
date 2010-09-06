#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmtiThreadState.hpp	1.20 03/12/23 16:43:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _JAVA_JVMTITHREADSTATE_H_
#define _JAVA_JVMTITHREADSTATE_H_

//
// Forward Declarations
//

class JvmdiCachedFrames;
class JvmtiEnvBase;
class JvmtiEnvThreadState;
class JvmtiDynamicCodeEventCollector;

///////////////////////////////////////////////////////////////
//
// class JvmtiThreadState
//
// The Jvmti state for each thread (across all JvmtiEnv):
// 1. Local table of enabled events.
class JvmtiThreadState : public CHeapObj {
 private:
  friend class JvmtiEnv;
  JavaThread        *_thread;
  bool              _exception_detected;
  bool              _exception_caught;
  bool              _hide_single_stepping;
  bool              _pending_step_for_popframe;
  int               _hide_level;

  // Used to send class being redefined info to the class  file load hook 
  // event handler. 
  KlassHandle 	    *_class_being_redefined;

  // This is only valid when is_interp_only_mode() returns true
  int               _cur_stack_depth;
  
  JvmtiThreadEventEnable _thread_event_enable;
  
  // for support of JvmtiEnvThreadState
  GrowableArray<JvmtiEnvThreadState *> *_env_thread_states;

  // doubly-linked linear list of active thread state
  // needed in order to iterate the list without holding Threads_lock
  static JvmtiThreadState *_head;
  JvmtiThreadState *_next;
  JvmtiThreadState *_prev;

  // holds cached jframeIDs for JVMDI clients - remove when JVMDI is removed
  JvmdiCachedFrames _jvmdi_cached_frames;

  // holds the current dynamic code event collector, NULL if no event collector in use
  JvmtiDynamicCodeEventCollector* _dynamic_code_event_collector;
  // holds the current vm object alloc event collector, NULL if no event collector in use
  JvmtiVMObjectAllocEventCollector* _vm_object_alloc_event_collector;

  // Should only be created by factory methods
  JvmtiThreadState(JavaThread *thread);
 public:
  ~JvmtiThreadState();

  // is event_type enabled and usable for this thread in any enviroments? 
  bool is_enabled(jvmtiEvent event_type) { 
    return _thread_event_enable.is_enabled(event_type); 
  }       

  JvmtiThreadEventEnable *thread_event_enable() {
    return &_thread_event_enable;
  }

  int env_count() {
    return _env_thread_states->length();
  }

  JvmtiEnvThreadState *env_thread_state(int index) {
    return _env_thread_states->at(index);
  }

  JvmtiEnvThreadState *env_thread_state(JvmtiEnvBase *env);

  void add_env(JvmtiEnvBase *env);
  void remove_env(JvmtiEnvBase *env);

  // Used by the interpreter for fullspeed debugging support
  bool is_interp_only_mode()                { return _thread->is_interp_only_mode(); }
  void enter_interp_only_mode();
  void leave_interp_only_mode();

  // access to the linked list of all JVMTI thread states
  static JvmtiThreadState *first() { 
    assert(Threads::number_of_threads() == 0 || JvmtiThreadState_lock->is_locked(), "sanity check");
    return _head; 
  }

  JvmtiThreadState *next()                  {
    return _next;
  }
                  
  // Current stack depth is only valid when is_interp_only_mode() returns true.
  // These functions should only be called at a safepoint - usually called from same thread.
  // Returns the number of Java activations on the stack.
  int cur_stack_depth();
  void invalidate_cur_stack_depth();
  void incr_cur_stack_depth();
  void decr_cur_stack_depth();

  int count_frames();

  inline JavaThread *get_thread()      { return _thread;              }
  inline bool is_exception_detected()  { return _exception_detected;  }
  inline bool is_exception_caught()    { return _exception_caught;  }
  inline void set_exception_detected() { _exception_detected = true; 
                                         _exception_caught = false; }
  inline void set_exception_caught()   { _exception_caught = true;
                                         _exception_detected = false; }

  inline void clear_hide_single_stepping() {
    if (_hide_level > 0) {
      _hide_level--;
    } else {
      assert(_hide_single_stepping, "hide_single_stepping is out of phase");
      _hide_single_stepping = false;
    }
  }
  inline bool hide_single_stepping() { return _hide_single_stepping; }
  inline void set_hide_single_stepping() {
    if (_hide_single_stepping) {
      _hide_level++;
    } else {
      assert(_hide_level == 0, "hide_level is out of phase");
      _hide_single_stepping = true;
    }
  }

  // Step pending flag is set when PopFrame is called and it is cleared
  // when step for the Pop Frame is completed.
  // This logic is used to distinguish b/w step for pop frame and repeat step.
  void set_pending_step_for_popframe() { _pending_step_for_popframe = true; }
  void clear_pending_step_for_popframe() { _pending_step_for_popframe = false; }
  bool is_pending_step_for_popframe() { return _pending_step_for_popframe; }
  void process_pending_step_for_popframe();   

  // Setter and getter method is used to send redefined class info
  // when class file load hook event is posted.
  // It is set while loading redefined class and cleared before the
  // class file load hook event is posted.
  inline void set_class_being_redefined(KlassHandle *h_class) {
    _class_being_redefined = h_class;
  }

  inline KlassHandle *get_class_being_redefined() {
    return _class_being_redefined;
  }

            
  // Todo: get rid of this!  
 private:
  bool _debuggable;
 public:
  // Should the thread be enumerated by jvmtiInternal::GetAllThreads?
  bool is_debuggable()                 { return _debuggable; }
  // If a thread cannot be suspended (has no valid last_java_frame) then it gets marked !debuggable
  void set_debuggable(bool debuggable) { _debuggable = debuggable; }

 public:

  // This should be used when a thread becomes not walkable
  // Used by both the current thread as well as suspend/resumption
  // (but not both at the same time).
  // Remove this and the whole mechanism that calls this when JVMDI is removed
  void clear_cached_frames() { 
    if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit()) {
      _jvmdi_cached_frames.clear_cached_frames(); 
    }
  }

  // Remove this when JVMDI is removed
  JvmdiCachedFrames* jvmdi_cached_frames()  { return &_jvmdi_cached_frames; }

  bool may_be_walked();

  // Thread local event collector setter and getter methods. 
  JvmtiDynamicCodeEventCollector* get_dynamic_code_event_collector() {
    return _dynamic_code_event_collector;
  } 
  JvmtiVMObjectAllocEventCollector* get_vm_object_alloc_event_collector() {
    return _vm_object_alloc_event_collector;
  } 
  void set_dynamic_code_event_collector(JvmtiDynamicCodeEventCollector* collector) {
    _dynamic_code_event_collector = collector;
  } 
  void set_vm_object_alloc_event_collector(JvmtiVMObjectAllocEventCollector* collector) {
    _vm_object_alloc_event_collector = collector;
  }
    

  //
  // Frame routines
  //

 public:

  //  true when the thread was suspended with a pointer to the last Java frame.
  bool has_last_frame()                     { return _thread->has_last_Java_frame(); }

  void update_for_pop_top_frame();

  // already holding JvmtiThreadState_lock - retrieve or create JvmtiThreadState
  inline static JvmtiThreadState *state_for_while_locked(JavaThread *thread) {
    assert(JvmtiThreadState_lock->is_locked(), "sanity check");

    JvmtiThreadState *state = thread->jvmti_thread_state();
    if (state == NULL) {
      state = new JvmtiThreadState(thread);
    }
    return state;
  }

  // retrieve or create JvmtiThreadState
  inline static JvmtiThreadState *state_for(JavaThread *thread) {
    JvmtiThreadState *state = thread->jvmti_thread_state();
    if (state == NULL) {
      MutexLocker mu(JvmtiThreadState_lock);
      // check again with the lock held
      state = state_for_while_locked(thread);
    }
    return state;
  }
};


#endif   /* _JAVA_JVMTITHREADSTATE_H_ */
