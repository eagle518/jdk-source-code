#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adapters_sparc.cpp	1.20 03/12/23 16:36:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_adapters_sparc.cpp.incl"

// sets up the C2I adapter when a deoptimization happens so that
// code can now be transferred to the interpreted frames
void C2IAdapter::unpack_c2i_adapter(frame stub_frame, frame adapter_frame, vframeArray* vframe_array) {

  Deoptimization::UnrollBlock* info = vframe_array->unroll_block();
  // The stub_frame is the deopt/uncommon_trap blob's frame.

  CodeBlob* stub_cb = CodeCache::find_blob(stub_frame.pc());
  assert(stub_cb->is_deoptimization_stub() || stub_cb->is_uncommon_trap_stub(), "just checking");
  assert(stub_cb->frame_size() >= 0, "Unexpected frame size");
  
  // NEEDS_CLEANUP should be able to remove these sp calculations and pick the value up from
  // a frame

  intptr_t *caller_sp = (stub_frame.sp() + stub_cb->frame_size()) + (info->size_of_frames()/BytesPerWord);
  assert(frame_size() >= 0, "Unexpected frame size");

  int callee_locals_size = (info->frame_sizes()[0]/wordSize - frame_size());
  assert(callee_locals_size == round_to(callee_locals_size, WordsPerLong), 
	 "must align");

#ifdef ASSERT
  // put a useless value in place to catch bugs
  *(adapter_frame.register_addr( IsavedSP )) = (intptr_t)-1;
#endif
  *(adapter_frame.register_addr( Lesp )) = (intptr_t)(caller_sp - 1);

  // Now fill in the values in the frame
  setup_stack_frame(adapter_frame, vframe_array);

}

// this function returns the adjust size (in number of words) to a c2i adapter
// activation for use during deoptimization
int Deoptimization::last_frame_adjust(int callee_parameters, int callee_locals ) {
  assert( callee_locals >= callee_parameters, 
	  "test and remove; got more parms than locals" );
  if( callee_locals < callee_parameters )
    return 0;			// No adjustment for negative locals
  return round_to((callee_locals - callee_parameters), WordsPerLong);
}

// Machine dependent code for computing the 'last_i2c_frame' adjust amount.
int vframeArray::i2c_frame_adjust( int callee_parameters, int callee_locals ) {
  int i2c_frame_adjust = 0;
  // On SPARC: only adjust the adapter's caller, an interpreted frame, when the
  // adapter is an i2c_adapter and not when it is an osr_adapter.
  if( adjust_adapter_caller() ) {
    i2c_frame_adjust = Deoptimization::last_frame_adjust(callee_parameters, callee_locals);
    _adapter_caller.set_sp( _adapter_caller.sp() - i2c_frame_adjust );
  }
  return i2c_frame_adjust;
}

//Reconciliation History
// 1.2 98/10/07 14:20:20 adapters_i486.cpp
// 1.3 98/11/15 18:22:55 adapters_i486.cpp
// 1.6 99/06/28 10:59:51 adapters_i486.cpp
//End

