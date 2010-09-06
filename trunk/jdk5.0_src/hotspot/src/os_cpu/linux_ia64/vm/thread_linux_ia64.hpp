#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_linux_ia64.hpp	1.14 04/03/03 16:48:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 *
 */

 private:

  void pd_initialize() {
    _anchor.clear();
    _last_interpreter_fp = NULL;
  }
  

  frame pd_last_frame() {
    assert(has_last_Java_frame(), "must have last_Java_sp() when suspended");
    return frame(last_Java_sp(), _anchor.last_Java_fp(), NULL, _anchor.last_Java_pc());
  }

 public:
  void set_last_Java_fp(intptr_t* fp) {
    _anchor.set_last_Java_fp(fp);
  }

  intptr_t* last_Java_fp(void) {
    return _anchor.last_Java_fp();
  }

  void set_base_of_stack_pointer(intptr_t* base_sp) {}

  static ByteSize last_Java_fp_offset()     { 
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_Java_fp_offset(); 
  }

  intptr_t* base_of_stack_pointer()   { return NULL; }
  void record_base_of_stack_pointer() {}

  // debugging support
  static frame current_frame_guess() { 
     warning("current_frame_guess unimplemented");
#if 0
    intptr_t* fp = (*CAST_TO_FN_PTR( intptr_t* (*)(void), StubRoutines::ia64::get_previous_fp_entry()))();
    frame fr(fp, frame::unpatchable, (address)NULL);
#endif
    frame fr;
    return fr;
  }

  int pd_get_fast_thread_id() { return (int) ThreadLocalStorage::thread(); }

  // Limits for stack checking.  Itanium has two different stacks
  // a register stack and a memory stack.  We must check both stacks
  // for overflows.  We store the address limits that we compare against here.

  protected:

  address _register_stack_base;
  address _register_stack_limit;
  address _memory_stack_limit;

  public:

  void set_register_stack_limit( address limit ) { _register_stack_limit = limit; }
  void set_register_stack_base( address base )   { _register_stack_base = base; }
  void set_memory_stack_limit( address limit )   { _memory_stack_limit = limit;   }
  address register_stack_limit()                 { return _register_stack_limit;  }
  address register_stack_base()                  { return _register_stack_base;  }
  address memory_stack_limit()                   { return _memory_stack_limit;  }

  static ByteSize register_stack_limit_offset() {
    return byte_offset_of(JavaThread, _register_stack_limit);
  }

  static ByteSize memory_stack_limit_offset() {
    return byte_offset_of(JavaThread, _memory_stack_limit);
  }

#if 0
  // Forte Analyzer AsyncGetCallTrace profiling support is not implemented
  // on Linux IA64 (yet).
  bool pd_get_top_frame_for_signal_handler(frame* fr_addr, void* ucontext,
    bool isInJava);
#endif

  // Routines to support the management of the register
  // stack on the Itanium processor.
  // register_stack_overflow returns true if it is unsafe to
  // unguard the register stack.
  static bool register_stack_overflow();
  static void enable_register_stack_guard();
  static void disable_register_stack_guard();

  // -Xprof support
  // 
  // In order to find the last Java fp from an async profile 
  // tick, we store the current interpreter fp in the thread.
  // This value is only valid while we are in the C++ interpreter 
  // and profiling.

  protected:

  intptr_t *_last_interpreter_fp;

  public:

  static ByteSize last_interpreter_fp_offset() {
    return byte_offset_of(JavaThread, _last_interpreter_fp);
  }

  intptr_t* last_interpreter_fp() { return _last_interpreter_fp; }
