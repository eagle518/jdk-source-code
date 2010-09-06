#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interp_masm_ia64.hpp	1.9 03/12/23 16:36:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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

  // Handy address generation macros
#define thread_(field_name) GR4_thread, in_bytes(JavaThread::field_name ## _offset())
#define method_(field_name) GR27_method, in_bytes(methodOopDesc::field_name ## _offset())
#define state_(field_name)  GR_Lstate, in_bytes(byte_offset_of(cInterpreter, field_name))
};

