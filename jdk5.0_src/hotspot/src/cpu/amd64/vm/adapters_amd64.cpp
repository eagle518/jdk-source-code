#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adapters_amd64.cpp	1.2 03/12/23 16:35:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_adapters_amd64.cpp.incl"

void C2IAdapter::unpack_c2i_adapter(frame stub_frame, 
                                    frame adapter_frame, 
                                    vframeArray* vframe_array)
{
  Deoptimization::UnrollBlock* info = vframe_array->unroll_block();

  // The stub_frame is the deopt/uncommon_trap blob's frame.

  CodeBlob* stub_cb = CodeCache::find_blob(stub_frame.pc());
  assert(stub_cb->is_deoptimization_stub() || 
         stub_cb->is_uncommon_trap_stub(), "just checking");
  assert(stub_cb->frame_size() >= 0, "Unexpected frame size");
  assert(frame_size() >= 0, "Unexpected frame size");

  // Now fill in the values in the frame
  setup_stack_frame(adapter_frame, vframe_array);
}

// this function returns the adjust size (in number of words) to a c2i
// adapter activation for use during deoptimization
int Deoptimization::last_frame_adjust(int callee_parameters,
                                      int callee_locals)
{
  return callee_locals - callee_parameters;
}

// Machine dependent code for computing the 'last_i2c_frame' adjust
// amount.
int vframeArray::i2c_frame_adjust(int callee_parameters, int callee_locals) 
{
  // Need size of I2C adapter to pop.  The I2C's sp starts where the
  // de-opting frame ends, at array->sp()+array->frame_size().  The
  // I2C's frame ends at the next frame's sp, adapter_caller().
  // Compute I2C's framesize as the diff.
  return adapter_caller().sp() - (sp() + frame_size());
}
