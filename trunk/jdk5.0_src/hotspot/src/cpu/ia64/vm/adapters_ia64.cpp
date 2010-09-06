#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adapters_ia64.cpp	1.7 03/12/23 16:36:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_adapters_ia64.cpp.incl"

// sets up the C2I adapter when a deoptimization happens so that
// code can now be transferred to the interpreted frames
void C2IAdapter::unpack_c2i_adapter(frame stub_frame, frame adapter_frame, vframeArray* vframe_array) {

  // The stub_frame is the deopt/uncommon_trap blob's frame.

  CodeBlob* stub_cb = CodeCache::find_blob(stub_frame.pc());
  assert(stub_cb->is_deoptimization_stub() || stub_cb->is_uncommon_trap_stub(), "just checking");
  assert(stub_cb->frame_size() >= 0, "Unexpected frame size");
  Deoptimization::UnrollBlock* info = vframe_array->unroll_block();

  // Now fill in the values in the frame
  setup_stack_frame(adapter_frame, vframe_array);

}

// this function returns the adjust size (in number of words) to a c2i adapter
// activation for use during deoptimization
int Deoptimization::last_frame_adjust(int callee_parameters, int callee_locals ) {
  assert( callee_locals >= callee_parameters, 
	  "test and remove; got more parms than locals" );
  // assert(false, "CHECK THIS");
  return callee_locals - callee_parameters;
}

// Machine dependent code for computing the 'last_i2c_frame' adjust amount.
int vframeArray::i2c_frame_adjust( int callee_parameters, int callee_locals ) {
  // assert(false, "CHECK THIS");
  return (0);
}

