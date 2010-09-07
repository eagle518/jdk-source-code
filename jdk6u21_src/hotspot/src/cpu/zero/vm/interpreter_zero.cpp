/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009, 2010 Red Hat, Inc.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreter_zero.cpp.incl"

address AbstractInterpreterGenerator::generate_slow_signature_handler() {
  _masm->advance(1);
  return (address) InterpreterRuntime::slow_signature_handler;
}

address InterpreterGenerator::generate_math_entry(
    AbstractInterpreter::MethodKind kind) {
  if (!InlineIntrinsics)
    return NULL;

  Unimplemented();
}

address InterpreterGenerator::generate_abstract_entry() {
  return ShouldNotCallThisEntry();
}

address InterpreterGenerator::generate_method_handle_entry() {
  return ShouldNotCallThisEntry();
}

bool AbstractInterpreter::can_be_compiled(methodHandle m) {
  return true;
}

int AbstractInterpreter::size_activation(methodOop method,
                                         int tempcount,
                                         int popframe_extra_args,
                                         int moncount,
                                         int callee_param_count,
                                         int callee_locals,
                                         bool is_top_frame) {
  return layout_activation(method,
                           tempcount,
                           popframe_extra_args,
                           moncount,
                           callee_param_count,
                           callee_locals,
                           (frame*) NULL,
                           (frame*) NULL,
                           is_top_frame);
}

void Deoptimization::unwind_callee_save_values(frame* f,
                                               vframeArray* vframe_array) {
}
