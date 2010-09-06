#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_win32_amd64.hpp	1.4 04/03/03 17:21:00 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

 private:
  void pd_initialize() {
    _anchor.clear();
  }

  frame pd_last_frame() {
    assert(has_last_Java_frame(), "must have last_Java_sp() when suspended");
    if (_anchor.last_Java_pc() != NULL) {
      return frame(_anchor.last_Java_sp(), _anchor.last_Java_fp(), _anchor.last_Java_pc());
    } else {
      // This will pick up pc from sp
      return frame(_anchor.last_Java_sp(), _anchor.last_Java_fp());
    }
  }


  int pd_get_fast_thread_id() {
#if 0
    int tib = 0;
    _asm {
      mov eax, dword ptr FS:[18H];
      mov tib, eax;
    }
    return tib;
#endif
    ShouldNotReachHere();
    return 0;
  }

 public:
  // Mutators are highly dangerous....
  intptr_t* last_Java_fp()                           { return _anchor.last_Java_fp(); }
  void  set_last_Java_fp(intptr_t* fp)               { _anchor.set_last_Java_fp(fp);   }

  void set_base_of_stack_pointer(intptr_t* base_sp)  {}


  static ByteSize last_Java_fp_offset()          { 
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_Java_fp_offset();
  }

  intptr_t* base_of_stack_pointer()                  { return NULL; }
  void record_base_of_stack_pointer()            {}

  // debugging support
  static frame current_frame_guess() { 
    intptr_t* fp = (*CAST_TO_FN_PTR( intptr_t* (*)(void), StubRoutines::amd64::get_previous_fp_entry()))();
    // fp points to the frame of the ps stub routine
    frame f(NULL, fp, (address)NULL);
    RegisterMap map(JavaThread::current(), false);
    return f.sender(&map);
  }

  bool pd_get_top_frame_for_signal_handler(frame* fr_addr, void* ucontext,
    bool isInJava);

  // These routines are only used on cpu architectures that
  // have separate register stacks (Itanium).
  static bool register_stack_overflow() { return false; }
  static void enable_register_stack_guard() {}
  static void disable_register_stack_guard() {}

