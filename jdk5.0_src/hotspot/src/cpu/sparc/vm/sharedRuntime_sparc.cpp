#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedRuntime_sparc.cpp	1.4 03/12/23 16:37:22 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_sparc.cpp.incl"

#define __ masm->

//
// Generate the on-stack replacement stub, that is used to replace the
// interpreter frame
//
OSRAdapter* SharedRuntime::generate_osr_blob(int frame_size) {
  ResourceMark rm;
  
  // setup code generation tools
  CodeBuffer*       cb = new CodeBuffer(128, 128, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(cb);    
  
  OopMapSet *oop_maps = new OopMapSet();
  // frame_size is in words, Oopmap want slots
  OopMap* map =  new OopMap(frame_size * (wordSize / sizeof(jint)), 0 );
  oop_maps->add_gc_map(0, true, map);
  
  // Continuation point after returning from osr compiled method.
  // Position a potential integer result for returning from the original interpreted activation.
  __ mov(O0, I0);
  __ mov(O1, I1);
  const Register Gtmp1 = G3_scratch ;

  // Return from the current method
  // The caller's SP was adjusted upon method entry to accomodate
  // the callee's non-argument locals. Undo that adjustment.
  __ ret();
  __ delayed()->restore(IsavedSP, G0, SP);

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size, 0);
}
