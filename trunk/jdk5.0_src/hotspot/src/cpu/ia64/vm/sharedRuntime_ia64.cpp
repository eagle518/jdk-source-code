#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedRuntime_ia64.cpp	1.5 03/12/23 16:36:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_ia64.cpp.incl"

#define __ _masm->

//
// Generate the on-stack replacement stub, that is used to replace the
// interpreter frame
//
OSRAdapter* SharedRuntime::generate_osr_blob(int frame_size) {
  ResourceMark rm;

  // setup code generation tools
  CodeBuffer*     cb    = new CodeBuffer(128, 64, 0, 0, 0, false);
  MacroAssembler* _masm = new MacroAssembler(cb);
  
  OopMapSet *oop_maps = new OopMapSet();
  OopMap* map =  new OopMap(frame_size, 0 );
  oop_maps->add_gc_map(0, true, map);

  __ breakm(2);

  // make sure all code is generated
  __ flush();

  warning("Kludge: OptoRuntime::generate_osr_blob");

  return OSRAdapter::new_osr_adapter(cb, oop_maps, frame_size, 0);
}

