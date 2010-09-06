#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreter_i486.hpp	1.21 03/12/23 16:36:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Interpreter: public AbstractInterpreter {
};



// Generation of Interpreter
//
// The InterpreterGenerator generates the interpreter into Interpreter::_code.
//
// After we enter a method and are executing the templates for each bytecode
// the following describes the register usage expected. This state is valid
// when we start to execute a bytecode and when we execute the next bytecode
// Note that eax/edx are special in the depending on the tosca they may or
// may not be live at entry/exit of the interpretation of a bytecode.
//
// eax: freely usable/caches tos
// ebx: freely usable
// ecx: freely usable
// edx: freely usable/caches tos
// edi: data index, points to beginning of locals section on stack
// esi: source index, points to beginning of bytecode (bcp)
// ebp: frame pointer
// esp: stack pointer (top-most element may be cached in registers)

class InterpreterGenerator: public AbstractInterpreterGenerator {
  friend class AbstractInterpreterGenerator;

 public:
  InterpreterGenerator(StubQueue* code);
 private:

  address generate_asm_interpreter_entry(bool synchronized);
  address generate_native_entry(bool synchronized);
  address generate_abstract_entry(void);
  address generate_math_entry(AbstractInterpreter::MethodKind kind);
  address generate_empty_entry(void);
  address generate_accessor_entry(void);
  static address frame_manager_return;
  static address frame_manager_sync_return;
  void lock_method(void);
  void generate_fixed_frame(bool native_call); // asm interpreter only
  void generate_stack_overflow_check(void);

#ifndef CORE
  void InterpreterGenerator::generate_counter_incr(Label* overflow, Label* profile_method, Label* profile_method_continue);
  void InterpreterGenerator::generate_counter_overflow(address entry_point);
  void generate_run_compiled_code(void);
  void check_for_compiled_code(Label & run_compiled_code);
#endif
};

