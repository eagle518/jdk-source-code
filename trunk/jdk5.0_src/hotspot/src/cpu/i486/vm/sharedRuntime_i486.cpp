#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedRuntime_i486.cpp	1.3 03/12/23 16:36:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_i486.cpp.incl"

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
  OopMap* map =  new OopMap(frame_size, 0 );
  OopMap* map2 = new OopMap(frame_size, 0 );
#ifdef COMPILER2
  // Create oopmap for osr adapter. All it contains is where to find the
  // link offset (ebp) on windows.
  int link_offset = ((frame_size - frame::sender_sp_offset) + frame::link_offset);
  map->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + link_offset), frame_size, 0, OptoReg::Name(EBP_num));
  map2->set_callee_saved(OptoReg::Name(SharedInfo::stack0 + link_offset), frame_size, 0, OptoReg::Name(EBP_num));
#endif
  oop_maps->add_gc_map(0, true, map);

  // Empty all except FPR0 in case of float/double returns
  __ ffree(0);
  int returning_fp_entry_offset = __ offset();
  oop_maps->add_gc_map(returning_fp_entry_offset, true, map2);
  for (int i = 1; i<8; i++ )
    __ ffree(i);

  __ movl(ecx, Address(ebp, frame::interpreter_frame_sender_sp_offset * wordSize)); // get sender sp
  __ leave();                                // remove frame anchor
  __ popl(esi);                              // get return address
  __ movl(esp, ecx);                         // set sp to sender sp
  __ jmp(esi);  
  __ flush();

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size, returning_fp_entry_offset);
}

