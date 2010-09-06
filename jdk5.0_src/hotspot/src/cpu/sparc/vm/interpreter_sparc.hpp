#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreter_sparc.hpp	1.19 03/12/23 16:37:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


class Interpreter: public AbstractInterpreter {
};


class InterpreterGenerator: AbstractInterpreterGenerator {
  friend class AbstractInterpreterGenerator;
 public:
  InterpreterGenerator(StubQueue* code);
 private:

#ifndef CORE
  void generate_check_compiled_code(Label &run_compiled_code);
#endif

  address generate_asm_interpreter_entry(bool synchronized);
  address generate_native_entry(bool synchronized);
  address generate_abstract_entry(void);
  address generate_math_entry(AbstractInterpreter::MethodKind kind);
  address generate_empty_entry(void);
  address generate_accessor_entry(void);
  static address frame_manager_return;
  static address frame_manager_sync_return;
  void lock_method(void);
  void save_native_result(void);
  void restore_native_result(void);
  void generate_fixed_frame(bool native_call); // asm interpreter only
  void generate_stack_overflow_check(Register Rframe_size, Register Rscratch,
                                     Register Rscratch2);

#ifndef CORE
  void generate_counter_incr(Label* overflow, Label* profile_method, Label* profile_method_continue);
  void generate_counter_overflow(bool native_call, Label& Lentry);
  void generate_run_compiled_code(void);
#endif
};
