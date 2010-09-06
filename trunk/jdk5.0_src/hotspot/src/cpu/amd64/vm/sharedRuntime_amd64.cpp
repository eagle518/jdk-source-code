#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sharedRuntime_amd64.cpp	1.4 03/12/23 16:35:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_amd64.cpp.incl"

#define __ masm->

//
// Generate the on-stack replacement stub, that is used to replace the
// interpreter frame
//
OSRAdapter* SharedRuntime::generate_osr_blob(int frame_size)
{
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer* cb = new CodeBuffer(128, 128, 0, 0, 0, false);
  MacroAssembler* masm = new MacroAssembler(cb);
  
  OopMapSet* oop_maps = new OopMapSet();
  OopMap* map = new OopMap(frame_size << (LogBytesPerWord - LogBytesPerInt),
                           0);

#ifdef COMPILER2
  // Create oopmap for osr adapter. All it contains is where to find
  // the link offset (rbp).
  int link_offset = 
    (frame_size - frame::sender_sp_offset + frame::link_offset) << 
    (LogBytesPerWord - LogBytesPerInt);

  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + link_offset),
                        frame_size << (LogBytesPerWord - LogBytesPerInt), 0,
                        OptoReg::Name(RBP_num));
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + link_offset + 1),
                        frame_size << (LogBytesPerWord - LogBytesPerInt), 0,
                        OptoReg::Name(RBP_H_num));
#endif

  oop_maps->add_gc_map(0, true, map);

  // get sender sp
  __ movq(rcx, 
          Address(rbp, 
                  frame::interpreter_frame_sender_sp_offset * wordSize));
  // remove frame anchor
  __ leave();
  // get return address
  __ popq(rscratch1);
  // set sp to sender sp
  __ movq(rsp, rcx);
  __ jmp(rscratch1);  
  __ flush();

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size, 0);
}

