#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)forte.cpp	1.44 04/04/06 07:01:40 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_forte.cpp.incl"


//-------------------------------------------------------

// Native interfaces for use by Forte tools.


#ifndef IA64

class vframeStreamForte : public vframeStreamCommon {
 public:
  // constructor that starts with sender of frame fr (top_frame)
  vframeStreamForte(JavaThread *jt, frame fr, bool stop_at_java_call_stub);
  void forte_next();
};


#ifndef CORE
static void forte_is_walkable_compiled_frame(frame* fr, RegisterMap* map,
  bool* is_compiled_p, bool* is_walkable_p);
#endif // !CORE
static bool forte_is_walkable_interpreted_frame(frame* fr,
  methodOop* method_p, int* bci_p);


// A Forte specific version of frame:safe_for_sender().
static bool forte_safe_for_sender(frame* fr, JavaThread *thread) {
  bool ret_value = false;  // be pessimistic

#ifdef COMPILER2
#if defined(IA32) || defined(AMD64)
  // On IA32 and AMD64, the standard safe_for_sender() version includes
  // a NULL check for the frame pointer (FP). This is a good thing when
  // the thread is executing Java code, but with Compiler2 it causes a
  // lot of valid frames to be dropped when we are in other thread states.
  if (thread->thread_state() == _thread_in_Java ||
      thread->thread_state() == _thread_in_Java_trans) {
#endif // IA32 || AMD64
#endif // COMPILER2

    ret_value = fr->safe_for_sender(thread);

#ifdef COMPILER2
#if defined(IA32) || defined(AMD64)
  } else {
    // This check is the same as the standard safe_for_sender()
    // on IA32 or AMD64 except that NULL FP values are tolerated.
    address   sp = (address)fr->sp();
    address   fp = (address)fr->fp();
    ret_value = sp != NULL && sp <= thread->stack_base() &&
      sp >= thread->stack_base() - thread->stack_size() &&
      (fp == NULL || (fp <= thread->stack_base() &&
      fp >= thread->stack_base() - thread->stack_size()));
  }
#endif // IA32 || AMD64
#endif // COMPILER2

  if (!ret_value) {
    return ret_value;  // not safe, nothing more to do
  }

#ifndef CORE
#ifdef SPARC
  // On Solaris SPARC, when a compiler frame has an interpreted callee
  // the _interpreter_sp_adjustment field contains the adjustment to
  // this frame's SP made by that interpreted callee.
  // For AsyncGetCallTrace(), we need to verify that the resulting SP
  // is valid for the specified thread's stack.
  address sp1 = (address)fr->sp();
  address sp2 = (address)fr->unextended_sp();

  // If the second SP is NULL, then the _interpreter_sp_adjustment
  // field simply adjusts this frame's SP to NULL and the frame is
  // not safe. This strange value can be set in the frame constructor
  // when our peek into the interpreted callee's adjusted value for
  // this frame's SP finds a NULL. This can happen when SIGPROF
  // catches us while we are creating the interpreter frame.
  //
  if (sp2 == NULL ||

      // If the two SPs are different, then _interpreter_sp_adjustment
      // is non-zero and we need to validate the second SP. We invert
      // the range check from frame::safe_for_sender() and bail out
      // if the second SP is not safe.
      (sp1 != sp2 && !(sp2 <= thread->stack_base()
      && sp2 >= (thread->stack_base() - thread->stack_size())))) {
    return false;
  }

  if (fr->is_entry_frame()) {
    // This frame thinks it is an entry frame; we need to validate
    // the JavaCallWrapper pointer.
    // Note: frame::entry_frame_is_first() assumes that the
    // JavaCallWrapper has a non-NULL _anchor field. We don't
    // check that here (yet) since we've never seen a failure
    // due to a NULL _anchor field.
    sp1 = (address)fr->entry_frame_call_wrapper();
    // We invert the range check from frame::safe_for_sender() and
    // bail out if the JavaCallWrapper * is not safe.
    if (!(sp1 <= thread->stack_base()
        && sp1 >= (thread->stack_base() - thread->stack_size()))) {
      return false;
    }
  }
#endif // SPARC
#endif // CORE

  return ret_value;
}


#ifndef CORE
// Unknown compiled frames have caused assertion failures on Solaris
// X86. This code also detects unknown compiled frames on Solaris
// SPARC, but no assertion failures have been observed. However, I'm
// paranoid so I'm enabling this code whenever we have a compiler.
//
// Returns true if the specified frame is an unknown compiled frame
// and false otherwise.
static bool is_unknown_compiled_frame(frame* fr, JavaThread *thread) {
  bool ret_value = false;  // be optimistic

  // This failure mode only occurs when the thread is in state
  // _thread_in_Java so we are okay for this check for any other
  // thread state.
  //
  // Note: _thread_in_Java does not always mean that the thread
  // is executing Java code. AsyncGetCallTrace() has caught
  // threads executing in JRT_LEAF() routines when the state
  // will also be _thread_in_Java.
  if (thread->thread_state() != _thread_in_Java) {
    return ret_value;
  }

  // This failure mode only occurs with compiled frames so we are
  // okay for this check for both entry and interpreted frames.
  if (fr->is_entry_frame() || fr->is_interpreted_frame()) {
    return ret_value;
  }

  // This failure mode only occurs when the compiled frame's PC
  // is in the code cache so we are okay for this check if the
  // PC is not in the code cache.
  CodeBlob* cb = CodeCache::find_blob(fr->pc()); 
  if (cb == NULL) {
    return ret_value;
  }

  // We have compiled code in the code cache so it is time for
  // the final check: let's see if any frame type is set
  ret_value = !(
    // is_entry_frame() is checked above
    // testers that are a subset of is_entry_frame():
    //   is_first_frame()
    fr->is_java_frame()
    // testers that are a subset of is_java_frame():
    //   is_interpreted_frame()
    //   is_compiled_frame()
    || fr->is_native_frame()
    || fr->is_runtime_frame()
    || fr->is_c2i_frame()
    || fr->is_i2c_frame()
    || fr->is_osr_adapter_frame()
    || fr->is_glue_frame()
    || fr->is_safepoint_blob_frame()
    );

  // If there is no frame type set, then we have an unknown compiled
  // frame and sender() should not be called on it.

  return ret_value;
}
#endif // !CORE


// top-frame will be skipped
vframeStreamForte::vframeStreamForte(JavaThread *jt, frame fr,
  bool stop_at_java_call_stub) : vframeStreamCommon(jt) {
  _stop_at_java_call_stub = stop_at_java_call_stub;

  // skip top frame, as it may not be at safepoint
  // For AsyncGetCallTrace(), we extracted as much info from the top
  // frame as we could in forte_is_walkable_frame(). We also verified
  // forte_safe_for_sender() so this sender() call is safe.
  _frame  = fr.sender(&_reg_map);

  if (jt->thread_state() == _thread_in_Java && !fr.is_first_frame()) {
    bool sender_check = false;  // assume sender is not safe

    if (forte_safe_for_sender(&_frame, jt)) {
      // If the initial sender frame is safe, then continue on with other
      // checks. The unsafe sender frame has been seen on Solaris X86
      // with both Compiler1 and Compiler2. It has not been seen on
      // Solaris SPARC, but seems like a good sanity check to have
      // anyway.

      // SIGPROF caught us in Java code and the current frame is not the
      // first frame so we should sanity check the sender frame. It is
      // possible for SIGPROF to catch us in the middle of making a call.
      // When that happens the current frame is actually a combination of
      // the real sender and some of the new call's info. We can't find
      // the real sender with such a current frame and things can get
      // confused.
      //
      // This sanity check has caught problems with the sender frame on
      // Solaris SPARC. So far Solaris X86 has not had a failure here.
      sender_check = _frame.is_entry_frame()
        // testers that are a subset of is_entry_frame():
        //   is_first_frame()
        || _frame.is_java_frame()
        // testers that are a subset of is_java_frame():
        //   is_interpreted_frame()
        //   is_compiled_frame()
        || _frame.is_native_frame()
#ifndef CORE
        || _frame.is_runtime_frame()
        || _frame.is_c2i_frame()
        || _frame.is_i2c_frame()
        || _frame.is_osr_adapter_frame()
        || _frame.is_glue_frame()
        || _frame.is_safepoint_blob_frame()
#endif // !CORE
        ;

      // We need an additional sanity check on an initial interpreted
      // sender frame. This interpreted frame needs to be both walkable
      // and have a valid BCI. This is yet another variant of SIGPROF
      // catching us in the middle of making a call.
      if (sender_check && _frame.is_interpreted_frame()) {
        methodOop method = NULL;
        int bci = -1;

        if (!forte_is_walkable_interpreted_frame(&_frame, &method, &bci)
            || bci == -1) {
          sender_check = false;
        }
      }

#ifndef CORE
      // We need an additional sanity check on an initial compiled
      // sender frame. This compiled frame also needs to be walkable.
      // This is yet another variant of SIGPROF catching us in the
      // middle of making a call.
      if (sender_check && !_frame.is_interpreted_frame()) {
        bool is_compiled, is_walkable;

        forte_is_walkable_compiled_frame(&_frame, &_reg_map,
          &is_compiled, &is_walkable);
        if (is_compiled && !is_walkable) {
          sender_check = false;
        }
      }
#endif // !CORE
    }

    if (!sender_check) {
      // nothing else to try if we can't recognize the sender
      _mode = at_end_mode;
      return;
    }
  }

  int loop_count = 0;
  int loop_max = MaxJavaStackTraceDepth * 2;

  while (!fill_from_frame()) {
    _frame = _frame.sender(&_reg_map);

    if (++loop_count >= loop_max) {
      // We have looped more than twice the number of possible
      // Java frames. This indicates that we are trying to walk
      // a stack that is in the middle of being constructed and
      // it is self referential.
      _mode = at_end_mode;
      return;
    }
  }
}


// Solaris SPARC Compiler1 needs an additional check on the grandparent
// of the top_frame when the parent of the top_frame is interpreted and
// the grandparent is compiled. However, in this method we do not know
// the relationship of the current _frame relative to the top_frame so
// we implement a more broad sanity check. When the previous callee is
// interpreted and the current sender is compiled, we verify that the
// current sender is also walkable. If it is not walkable, then we mark
// the current vframeStream as at the end.
void vframeStreamForte::forte_next() {
  // handle frames with inlining
#ifndef CORE
  if (_mode == compiled_mode &&
      vframeStreamCommon::fill_in_compiled_inlined_sender()) {
    return;
  }
#endif // !CORE

  // handle general case
  nmethod* code = _mode == compiled_mode ? _code : NULL;

  int loop_count = 0;
  int loop_max = MaxJavaStackTraceDepth * 2;

  do {
#ifndef CORE
#if defined(COMPILER1) && defined(SPARC)
    // remember if the current callee is interpreted
    bool prevIsInterpreted = _frame.is_interpreted_frame();
#endif // COMPILER1 && SPARC
#endif // !CORE

    _frame = _frame.sender(&_reg_map, (CodeBlob*)code);    

#ifndef CORE
#if defined(COMPILER1) && defined(SPARC)
    if (prevIsInterpreted) {
      // previous callee was interpreted and may require a special check
      bool scratch;
      if (_frame.is_compiled_frame(&scratch)) {
        // compiled sender called interpreted callee so need one more check
        bool is_compiled, is_walkable;

        // sanity check the compiled sender frame
        forte_is_walkable_compiled_frame(&_frame, &_reg_map,
          &is_compiled, &is_walkable);
        assert(is_compiled, "sanity check");
        if (!is_walkable) {
          // compiled sender frame is not walkable so bail out
          _mode = at_end_mode;
          return;
        }
      }
    }
#endif // COMPILER1 && SPARC
#endif // !CORE

    code = NULL; // After first iteration, we got no cached nmethod

    if (++loop_count >= loop_max) {
      // We have looped more than twice the number of possible
      // Java frames. This indicates that we are trying to walk
      // a stack that is in the middle of being constructed and
      // it is self referential.
      _mode = at_end_mode;
      return;
    }
  } while (!fill_from_frame());
}


// is_valid_method() exists in fprofiler.cpp, jvmpi.cpp and now here.
// We need one central version of this routine.

static inline bool forte_is_valid_method(methodOop method) {
  if (method == NULL || 
      // The methodOop is extracted via an offset from the current
      // interpreter frame. With AsyncGetCallTrace() the interpreter
      // frame may still be under construction so we need to make
      // sure that we got an aligned oop before we try to use it.
      !Space::is_aligned(method) ||
      !Universe::heap()->is_in(method) ||
      // See if GC became active after we entered AsyncGetCallTrace()
      // and before we try to use the methodOop. This routine is
      // used in validation of the top_frame so we don't have any
      // other data to flush if we bail due to GC here.
      // Yes, there is still a window after this check and before
      // we use methodOop below, but we can't lock out GC so that
      // has to be an acceptable risk.
      Universe::heap()->is_gc_active() ||
      // klass() does not need vtable dispatch so check before is_perm()
      oop(method)->klass() != Universe::methodKlassObj() ||
      !method->is_perm() || 
      !method->is_method()) {
    return false;   // doesn't look good
  }
  return true;      // hopefully this is a method indeed
}


#ifndef CORE
// Determine if 'fr' is a walkable, compiled frame.
// *is_compiled_p is set to true if the frame is compiled and if it
// is, then *is_walkable_p is set to true if it is also walkable.
static void forte_is_walkable_compiled_frame(frame* fr, RegisterMap* map,
  bool* is_compiled_p, bool* is_walkable_p) {

  *is_compiled_p = false;
  *is_walkable_p = false;

  CodeBlob* cb = CodeCache::find_blob(fr->pc()); 
  if (cb != NULL && cb->is_java_method()) {
    // frame is compiled and executing a Java method
    *is_compiled_p = true;

    bool at_call = map->is_pc_at_call(fr->id());
    PcDesc* pc_desc = ((nmethod*) cb)->pc_desc_at(fr->pc(), at_call);       
    if (pc_desc != NULL) {
      // it has a PcDesc so the frame is also walkable
      *is_walkable_p = true;
    }
    // Implied else: this compiled frame has no PcDesc, i.e., contains
    // a frameless stub such as C1 method exit, so it is not walkable.
  }
  // Implied else: this isn't a compiled frame so it isn't a
  // walkable, compiled frame.
}
#endif // !CORE


// Determine if 'fr' is a walkable interpreted frame. Returns false
// if it is not. *method_p, and *bci_p are not set when false is
// returned. *method_p is non-NULL if frame was executing a Java
// method. *bci_p is != -1 if a valid BCI in the Java method could
// be found.
// Note: this method returns true when a valid Java method is found
// even if a valid BCI cannot be found.

static bool forte_is_walkable_interpreted_frame(frame* fr,
  methodOop* method_p, int* bci_p) {
  assert(fr->is_interpreted_frame(), "just checking");

  // top frame is an interpreted frame 
  // check if it is walkable (i.e. valid methodOop and valid bci)
  if (fr->is_interpreted_frame_valid()) {
    if (fr->fp() != NULL) {
      // access address in order not to trigger asserts that
      // are built in interpreter_frame_method function
      methodOop method = *fr->interpreter_frame_method_addr();
      if (forte_is_valid_method(method)) {
        int bci = -1;

        intptr_t bcx = fr->interpreter_frame_bcx();
        if (!method->is_native() && (frame::is_bci(bcx) || method->contains((address) bcx))) {
          bci = fr->interpreter_frame_bci();
          // code_size() may return 0 and we allow 0 here
          if (!(bci == 0 || bci >= 0 && bci < method->code_size())) {
            // reset bci to -1 if not a valid bci
            bci = -1;
          }
        }

        *method_p = method;
        *bci_p = bci;
        return true;
      }
    }
  }
  return false;
}


// Forte specific version of jvmpi.cpp: is_walkable_frame()
//
// Determine if 'fr' can be used to find a walkable frame. Returns
// false if a walkable frame cannot be found. *walkframe_p, *method_p,
// and *bci_p are not set when false is returned. Returns true if a
// walkable frame is returned via *walkframe_p. *method_p is non-NULL
// if the returned frame was executing a Java method. *bci_p is != -1
// if a valid BCI in the Java method could be found.
//
// *walkframe_p will be used by vframeStreamForte as the initial
// frame for walking the stack. Currently the initial frame is
// skipped by vframeStreamForte because we inherited the logic from
// the vframeStream class. This needs to be revisited in the future.
static bool forte_is_walkable_frame(JavaThread* thread, frame* fr,
  frame* walkframe_p, methodOop* method_p, int* bci_p) {

  if (!forte_safe_for_sender(fr, thread)
#ifndef CORE
      || is_unknown_compiled_frame(fr, thread)
#endif // !CORE
     ) {
    // If the initial frame is not safe, then bail out. So far this
    // has only been seen on Solaris X86 with Compiler2, but it seems
    // like a great initial sanity check.
    return false;
  }

  if (fr->is_first_frame()) {
    // If initial frame is frame from StubGenerator and there is no
    // previous anchor, there are no java frames yet
    return false;
  }

  if (fr->is_interpreted_frame()) {
    if (forte_is_walkable_interpreted_frame(fr, method_p, bci_p)) {
      *walkframe_p = *fr;
      return true;
    }
    return false;
  }

  // At this point we have something other than a first frame or an
  // interpreted frame.

  methodOop method = NULL;

#ifndef CORE
  // Determine if this top frame is executing a Java method.
  if (CodeCache::contains(fr->pc())) {
    // top frame is a compiled frame or stub routines
    CodeBlob* cb = CodeCache::find_blob(fr->pc());

#ifdef COMPILER2
#if defined(IA32) || defined(AMD64)
    if (cb->is_i2c_adapter()) {
      // In Compiler2 for IA32, sender_for_compiled_frame() assumes
      // that an i2c frame is well formed and has the sender's SP at
      // the end of the frame. In the normal system, this is a valid
      // assumption. However, with AsyncGetCallTrace(), a SIGPROF
      // could have arrived while we were creating the i2c frame. We
      // need to verify that the sender's SP before using it.

      // This initial sender_sp calculation is common to more than
      // just i2c frames, but we have only seen problems with i2c
      // so we limit the scope of the check.
      intptr_t* sender_sp = fr->sp() + cb->frame_size();

      // hijack one check from frame::safe_for_sender()
      address check_sp = (address)sender_sp;
      if (check_sp <= thread->stack_base() &&
          check_sp >= thread->stack_base() - thread->stack_size()) {

        // this sender_sp adjustment retrieves the assumed sender's
        // SP from the end of the frame
        sender_sp = (intptr_t*)(*(sender_sp - 1)) + 1;

        // invert same check from frame::safe_for_sender()
        check_sp = (address)sender_sp;
        if (!(check_sp <= thread->stack_base() &&
            check_sp >= thread->stack_base() - thread->stack_size())) {
          // proposed sender SP doesn't work so bail
          return false;
        }
      } else {
        // current frame size doesn't work for current SP so bail
        return false;
      }
    }

    // In Compiler2 for IA32, sender_for_compiled_frame() assumes that
    // a compiled frame is well formed and has the sender's FP at a
    // well-defined offset from the current SP. In the normal system,
    // this is a valid assumption. However, with AsyncGetCallTrace(),
    // a SIGPROF could have arrived while we were creating the compiled
    // frame. We need to verify that the sender's FP before using it.
    int llink_offset = cb->link_offset();
    if (llink_offset >= 0) {
      address check_fp = (address)(fr->sp() + llink_offset);
      // hijack and invert one check from frame::safe_for_sender()
      if (!(check_fp <= thread->stack_base() &&
          check_fp >= thread->stack_base() - thread->stack_size())) {
        // proposed sender FP doesn't work so bail
        return false;
      }
    }
#endif // IA32 || AMD64
#endif // COMPILER2

    if (cb->is_nmethod()) {
      method = ((nmethod *)cb)->method();
    }
  }

  // If the first java frame is a compiled frame and it does not have a PcDesc.
  // then the stack is not walkable (detected below).  Otherwise, it is walkable.
  RegisterMap map(thread, false);
  frame first_java_frame;

  {
    // This is a copy of frame::profile_find_Java_sender_frame().
    // We need access to the RegisterMap and we only want to get
    // it once.

    // Find the first Java frame on the stack
    if (fr->is_java_frame()) {
      // top frame is compiled frame
      first_java_frame = *fr;
    }
    // For AsyncGetCallTrace() we cannot assume there is a sender
    // for the initial frame. The initial forte_safe_for_sender() call
    // and check for is_first_frame() is done on entry to this method.
    else {
      int loop_count = 0;
      int loop_max = MaxJavaStackTraceDepth * 2;

      for (frame sender_frame = fr->sender(&map);
        forte_safe_for_sender(&sender_frame, thread)
        && !sender_frame.is_first_frame()
#ifndef CORE
        && !is_unknown_compiled_frame(&sender_frame, thread)
#endif // !CORE
        ; sender_frame = sender_frame.sender(&map)) {

        if (sender_frame.is_java_frame()) {
          first_java_frame = sender_frame;
          break;
        }

        if (++loop_count >= loop_max) {
          // We have looped more than twice the number of possible
          // Java frames. This indicates that we are trying to walk
          // a stack that is in the middle of being constructed and
          // it is self referential. So far this problem has only
          // been seen on Solaris X86 Compiler2, but it seems like
          // a good robustness fix for all platforms.
          break;
        }
      }
    }
  }

  if (first_java_frame.sp() != NULL && first_java_frame.pc() != NULL) {
    // If we found the first Java frame on the stack in the code
    // block above, then we need to sanity check it.
    bool is_compiled, is_walkable;

    forte_is_walkable_compiled_frame(&first_java_frame, &map,
      &is_compiled, &is_walkable);
    if (is_compiled && !is_walkable) {
      // The compiled Java frame we found is not walkable so we cannot
      // return fr as our top-most frame. We return the compiled Java
      // frame that we found as the top-most frame in order to return
      // the method info that we also found. This is "safe" because
      // the top-most frame is skipped in subsequent stack walking.
      *walkframe_p = first_java_frame;
      *method_p = method;
      *bci_p = -1;
      return true;
    }
  }
#endif // !CORE

  *walkframe_p = *fr;
  *method_p = method;
  *bci_p = -1;
  return true;
}


//
// Forte specific version of jvmpi.cpp:fill_call_trace_given_top()
//
static void forte_fill_call_trace_given_top(JavaThread* thd,
  JVMPI_CallTrace* trace, int depth, frame top_frame) {
  ResetNoHandleMark rnhm;

  frame walkframe;
  methodOop method;
  int bci;
  int count;

  count = 0;
  assert(trace->frames != NULL, "trace->frames must be non-NULL");

  if (!forte_is_walkable_frame(thd, &top_frame, &walkframe, &method, &bci)) {
    // return if no walkable frame is found
    return;
  }

  if (method != NULL) {
    // The method is not stored GC safe so see if GC became active
    // after we entered AsyncGetCallTrace() and before we try to
    // use the methodOop.
    // Yes, there is still a window after this check and before
    // we use methodOop below, but we can't lock out GC so that
    // has to be an acceptable risk.
    if (!forte_is_valid_method(method)) {
      trace->num_frames = -2;
      return;
    }

    count++;
    trace->num_frames = count;
    trace->frames[0].method_id = method->find_jmethod_id_or_null();
    if (!method->is_native()) {
      trace->frames[0].lineno = bci;
    } else {
      trace->frames[0].lineno = -3;
    }
  } 

  // check has_last_Java_frame() after looking at the top frame
  // which may be an interpreted Java frame.
  if (!thd->has_last_Java_frame() && count == 0) {
    trace->num_frames = 0;
    return;
  }

  vframeStreamForte st(thd, walkframe, false);
  for (; !st.at_end() && count < depth; st.forte_next(), count++) {
    bci = st.bci();
    method = st.method();

    // The method is not stored GC safe so see if GC became active
    // after we entered AsyncGetCallTrace() and before we try to
    // use the methodOop.
    // Yes, there is still a window after this check and before
    // we use methodOop below, but we can't lock out GC so that
    // has to be an acceptable risk.
    if (!forte_is_valid_method(method)) {
      // we throw away everything we've gathered in this sample since
      // none of it is safe
      trace->num_frames = -2;
      return;
    }

    trace->frames[count].method_id = method->find_jmethod_id_or_null();
    if (!method->is_native()) {
      trace->frames[count].lineno = bci;
    } else {
      trace->frames[count].lineno = -3;
    }
  }
  trace->num_frames = count;
  return;
}


// Forte Analyzer AsyncGetCallTrace() entry point. Currently supported
// on Linux X86, Solaris SPARC and Solaris X86.
//
// Async-safe version of GetCallTrace being called from a signal handler
// when a LWP gets interrupted by SIGPROF but the stack traces are filled
// with different content (see below).
// 
// This function must only be called when either JVM/PI or JVM/TI
// CLASS_LOAD events have been enabled since agent startup. The enabled
// event will cause the jmethodIDs to be allocated at class load time.
// The jmethodIDs cannot be allocated in a signal handler because locks
// cannot be grabbed in a signal handler safely.
//
// void (*AsyncGetCallTrace)(JVMPI_CallTrace *trace, jint depth, void* ucontext)
//
// Called by the profiler to obtain the current method call stack trace for 
// a given thread. The thread is identified by the env_id field in the 
// JVMPI_CallTrace structure. The profiler agent should allocate a JVMPI_CallTrace 
// structure with enough memory for the requested stack depth. The VM fills in 
// the frames buffer and the num_frames field. 
//
// Arguments: 
//
//   trace    - trace data structure to be filled by the VM. 
//   depth    - depth of the call stack trace. 
//   ucontext - ucontext_t of the LWP
//
// JVMPI_CallTrace:
//   typedef struct {
//       JNIEnv *env_id;
//       jint num_frames;
//       JVMPI_CallFrame *frames;
//   } JVMPI_CallTrace;
//
// Fields:
//   env_id     - ID of thread which executed this trace. 
//   num_frames - number of frames in the trace. 
//                (< 0 indicates the frame is not walkable).
//   frames     - the JVMPI_CallFrames that make up this trace. Callee followed by callers.
//
//  JVMPI_CallFrame:
//    typedef struct {
//        jint lineno;                     
//        jmethodID method_id;              
//    } JVMPI_CallFrame;
//
//  Fields: 
//    1) For Java frame (interpreted and compiled),
//       lineno    - bci of the method being executed or -1 if bci is not available
//       method_id - jmethodID of the method being executed
//    2) For native method
//       lineno    - (-3)
//       method_id - jmethodID of the method being executed

extern "C" {
void AsyncGetCallTrace(JVMPI_CallTrace *trace, jint depth, void* ucontext) {
  if (SafepointSynchronize::is_synchronizing()) {
    // The safepoint mechanism is trying to synchronize all the threads.
    // Since this can involve thread suspension, it is not safe for us
    // to be here. We can reduce the deadlock risk window by quickly
    // returning to the SIGPROF handler. However, it is still possible
    // for VMThread to catch us here or in the SIGPROF handler. If we
    // are suspended while holding a resource and another thread blocks
    // on that resource in the SIGPROF handler, then we will have a
    // three-thread deadlock (VMThread, this thread, the other thread).
    trace->num_frames = -10;
    return;
  }

  JavaThread* thread;

  if (trace->env_id == NULL ||
    (thread = JavaThread::thread_from_jni_environment(trace->env_id)) == NULL ||
    thread->is_exiting()) {

    // bad env_id, thread has exited or thread is exiting
    trace->num_frames = -8;
    return;
  }

  if (thread->in_deopt_handler()) {
    // thread is in the deoptimization handler so return no frames
    trace->num_frames = -9;
    return;
  }

  assert(JavaThread::current() == thread, 
         "AsyncGetCallTrace must be called by the current interrupted thread");

  if (!JvmtiExport::should_post_class_load() &&
      // check JVM/PI after JVM/TI since JVM/TI is now preferred
      !jvmpi::is_event_enabled(JVMPI_EVENT_CLASS_LOAD)) {
    trace->num_frames = -1;
    return;
  }

  if (Universe::heap()->is_gc_active()) {
    trace->num_frames = -2;
    return;
  }

  switch (thread->thread_state()) {
  case _thread_new:
  case _thread_uninitialized:
  case _thread_new_trans:
    // We found the thread on the threads list above, but it is too
    // young to be useful so return that there are no Java frames.
    trace->num_frames = 0;
    break;
  case _thread_in_native:
  case _thread_in_native_trans:
  case _thread_blocked:
  case _thread_blocked_trans:
  case _thread_in_vm:
  case _thread_in_vm_trans:
    {
      frame fr;
      
      // param isInJava == false - indicate we aren't in Java code
      if (!thread->pd_get_top_frame_for_signal_handler(&fr, ucontext, false)) {
        if (!thread->has_last_Java_frame()) {
          trace->num_frames = 0;   // no Java frames
        } else {
          trace->num_frames = -3;  // unknown frame
        }
      } else {
        trace->num_frames = -4;    // non walkable frame by default
        forte_fill_call_trace_given_top(thread, trace, depth, fr);
      }      
    }
    break;
  case _thread_in_Java: 
  case _thread_in_Java_trans: 
    {
      frame fr;

      // param isInJava == true - indicate we are in Java code
      if (!thread->pd_get_top_frame_for_signal_handler(&fr, ucontext, true)) {
        trace->num_frames = -5;  // unknown frame
      } else {
        trace->num_frames = -6;  // non walkable frame by default
        forte_fill_call_trace_given_top(thread, trace, depth, fr);
      }
    }
    break;
  default:
    // Unknown thread state
    trace->num_frames = -7;
    break;
  }
}


#ifndef _WINDOWS
// Support for the Forte(TM) Peformance Tools collector.
//
// The method prototype is derived from libcollector.h. For more
// information, please see the libcollect man page.

// Method to let libcollector know about a dynamically loaded function.
// Because it is weakly bound, the calls become NOP's when the library
// isn't present.
void    collector_func_load(char* name,
                            void* null_argument_1,
                            void* null_argument_2,
                            void *vaddr,
                            int size,
                            int zero_argument,
                            void* null_argument_3);
#pragma weak collector_func_load
#define collector_func_load(x0,x1,x2,x3,x4,x5,x6) \
        ( collector_func_load ? collector_func_load(x0,x1,x2,x3,x4,x5,x6),0 : 0 )
#endif // !_WINDOWS

} // end extern "C"
#endif // !IA64

void Forte::register_stub(const char* name, address start, address end) {
#if !defined(_WINDOWS) && !defined(IA64)
  if (!jvmpi::enabled()) {
    // no agent is attached so there is nothing to do
    return;
  }

  assert(pointer_delta(end, start, sizeof(jbyte)) < INT_MAX,
    "Code size exceeds maximum range")

  collector_func_load((char*)name, NULL, NULL, start,
    pointer_delta(end, start, sizeof(jbyte)), 0, NULL);
#endif // !_WINDOWS && !IA64
}
