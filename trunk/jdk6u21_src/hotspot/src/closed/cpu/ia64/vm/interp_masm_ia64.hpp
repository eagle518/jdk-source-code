/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file specializes the assember with interpreter-specific macros

class InterpreterMacroAssembler: public MacroAssembler {

 public:
  InterpreterMacroAssembler(CodeBuffer* code) : MacroAssembler(code) {}

  // Counters
  void increment_invocation_counter(const Register counter);

  // Object locking
  void lock_object  (Register monitor_addr, Register obj_addr);
  void unlock_object(Register monitor_addr);

  // support for jvmdi/jvmpi
  void notify_method_entry();
  void notify_method_exit();

  void push_tag(Register tos, frame::Tag tag);

  // Handy address generation macros
#define thread_(field_name) GR4_thread, in_bytes(JavaThread::field_name ## _offset())
#define method_(field_name) GR27_method, in_bytes(methodOopDesc::field_name ## _offset())
#define state_(field_name)  GR_Lstate, in_bytes(byte_offset_of(cInterpreter, field_name))
};
