#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreter_ia64.hpp	1.15 03/12/23 16:36:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Used to provide a false return address for recursive C++ interpreter entries.
extern "C" void InterpretMethodDummy(interpreterState istate);
// Used to provide a false return address for initial C++ interpreter entry.
extern "C" void InterpretMethodInitialDummy(void);

class Interpreter: public AbstractInterpreter {
  public:
  static bool contains(address pc)            { 
    address* dummy_start = CAST_FROM_FN_PTR(address*, InterpretMethodDummy);
    address* dummy_start2 = CAST_FROM_FN_PTR(address*, InterpretMethodInitialDummy);
    address* im_start = CAST_FROM_FN_PTR(address*, cInterpreter::InterpretMethod);
    address* im_end = CAST_FROM_FN_PTR(address*, cInterpreter::End_Of_Interpreter);
    return (  AbstractInterpreter::contains(pc) || 
	      ( pc == *dummy_start + frame::pc_return_offset) || 
	      ( pc == *dummy_start2 + frame::pc_return_offset) || 
	      ( pc >= *im_start && pc < *im_end));
  }
};


class InterpreterGenerator: AbstractInterpreterGenerator {
  friend class AbstractInterpreterGenerator;

 public:
  InterpreterGenerator(StubQueue* code);
  // Above would be true if we looked for  istate in O0
  static int is_dummy_frame(address pc) { return (pc ==  (CAST_FROM_FN_PTR(address, InterpretMethodInitialDummy)+8) ); }
  // QQQ this is way too complicated
  static int is_interpreter_return(address pc) { return (Interpreter::contains(pc) && !is_dummy_frame(pc)); }

 private:
  address generate_interpreter_frame_manager(void);
  void generate_compute_interpreter_state(Label& exception_return);
  address generate_native_entry(void);
  address generate_abstract_entry(void);
  address generate_math_entry(AbstractInterpreter::MethodKind kind);
  address generate_empty_entry(void);
  address generate_accessor_entry(void);
  void lock_method(void);
  void unlock_method(void);
#ifndef CORE
  void generate_counter_incr(Label& overflow);
  void generate_counter_overflow(Label& retry_entry);
  void generate_run_compiled_code(void);
  void generate_check_compiled_code(Label& run_compiled_code);
#endif
};
