#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaFrameAnchor.hpp	1.8 03/12/23 16:43:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
//
// An object for encapsulating the machine/os dependent part of a JavaThread frame state
//
class JavaThread;

class JavaFrameAnchor VALUE_OBJ_CLASS_SPEC {
// Too many friends...
friend class CallNativeDirectNode;
friend class OptoRuntime;
friend class Runtime1;
friend class StubAssembler;
friend class CallRuntimeDirectNode;
friend class MacroAssembler;
friend class InterpreterGenerator;
friend class LIR_Assembler;
friend class GraphKit;
friend class StubGenerator;
friend class JavaThread;
friend class frame;
friend class VMStructs;
friend class cInterpreter;
friend class JavaCallWrapper;

 private:
   enum Constants {
     not_at_call_flag  = 2                        // Used to mark existent of a not_at_call frame
   };

  //
  // Whenever _last_Java_sp != NULL other anchor fields MUST be valid!
  // The stack may not be walkable [check with walkable() ] but the values must be valid.
  // The profiler apparently depends on this.
  //
  intptr_t* volatile _last_Java_sp;

  // Whenever we call from Java to native we can not be assured that the return
  // address that composes the last_Java_frame will be in an accessible location
  // so calls from Java to native store that pc (or one good enough to locate
  // the oopmap) in the frame anchor. Since the frames that call from Java to
  // native are never deoptimized we never need to patch the pc and so this
  // is acceptable.
  volatile  address _last_Java_pc;
  intptr_t* volatile _not_at_call_id;
  volatile int _flags;

  // tells whether the last Java frame is set
  // It is important that when last_Java_sp != NULL that the rest of the frame
  // anchor (including platform specific) all be valid.

  bool has_last_Java_frame() const                   { return _last_Java_sp != NULL; }
  // This is very dangerous unless sp == NULL
  // Invalidate the anchor so that has_last_frame is false
  // and no one should look at the other fields.
  void zap(void)                                     { _last_Java_sp = NULL; }
  
  // scope-desc/oopmap lookup 
  void  set_pc_not_at_call_for_frame(intptr_t* id)      {
    _not_at_call_id = id;
    _flags |= not_at_call_flag;
  }

  bool  not_at_call_frame_exists() const                { return ( _flags & not_at_call_flag) ; }

  intptr_t* not_at_call_id() const                      { return not_at_call_frame_exists() ? (intptr_t*)_not_at_call_id : NULL; }


#include "incls/_javaFrameAnchor_pd.hpp.incl"

public:
  JavaFrameAnchor()                              { clear(); }
  JavaFrameAnchor(JavaFrameAnchor *src)          { copy(src); }

  address last_Java_pc(void)                     { return _last_Java_pc; }
  void set_last_Java_pc(address pc)              { _last_Java_pc = pc; }

  int flags(void)                                { return _flags; }
  void set_flags(int flags)                      { _flags = flags; }

  // Assembly stub generation helpers

  static ByteSize last_Java_sp_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_sp); }
  static ByteSize last_Java_pc_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_pc); }
  static ByteSize flags_offset()                 { return byte_offset_of(JavaFrameAnchor, _flags); }

};

