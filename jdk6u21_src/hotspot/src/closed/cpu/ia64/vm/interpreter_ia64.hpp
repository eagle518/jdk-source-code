/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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

  // Support for Tagged Stacks

  // Stack index relative to tos (which points at value)
  static int expr_index_at(int i)     {
    return stackElementWords() * i;
  }
  static int expr_tag_index_at(int i) {
    assert(TaggedStackInterpreter, "should not call this");
    // tag is one word above java stack element
    return stackElementWords() * i + 1;
  }

  // Already negated by c++ interpreter
  static int local_index_at(int i)     {
    assert(i<=0, "local direction already negated");
    return stackElementWords() * i + (value_offset_in_bytes()/wordSize);
  }

  static int local_tag_index_at(int i) {
    assert(i<=0, "local direction already negated");
    assert(TaggedStackInterpreter, "should not call this");
    return stackElementWords() * i + (tag_offset_in_bytes()/wordSize);
  }

  // Number is taken from the existing code for ia64.
  // It's probably too big.
  const static int InterpreterCodeSize = 256 * 1024;
};


class InterpreterGenerator: AbstractInterpreterGenerator {
  friend class AbstractInterpreterGenerator;

 public:
  InterpreterGenerator(StubQueue* code);
  // Above would be true if we looked for  istate in O0
  static int is_dummy_frame(address pc) { return (pc ==  (CAST_FROM_FN_PTR(address, InterpretMethodInitialDummy)+8) ); }
  // this is way too complicated
  static bool is_interpreter_return(address pc) { return (Interpreter::contains(pc) && !is_dummy_frame(pc)); }

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
  void generate_counter_incr(Label& overflow);
  void generate_counter_overflow(Label& continue_entry);
};
