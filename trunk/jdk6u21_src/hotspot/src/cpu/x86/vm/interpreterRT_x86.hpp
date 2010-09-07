/*
 * Copyright (c) 1998, 2005, Oracle and/or its affiliates. All rights reserved.
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

// native method calls

class SignatureHandlerGenerator: public NativeSignatureIterator {
 private:
  MacroAssembler* _masm;
#ifdef AMD64
#ifdef _WIN64
  unsigned int _num_args;
#else
  unsigned int _num_fp_args;
  unsigned int _num_int_args;
#endif // _WIN64
  int _stack_offset;
#else
  void move(int from_offset, int to_offset);
  void box(int from_offset, int to_offset);
#endif // AMD64

  void pass_int();
  void pass_long();
  void pass_float();
#ifdef AMD64
  void pass_double();
#endif // AMD64
  void pass_object();

 public:
  // Creation
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer) : NativeSignatureIterator(method) {
    _masm = new MacroAssembler(buffer);
#ifdef AMD64
#ifdef _WIN64
    _num_args = (method->is_static() ? 1 : 0);
    _stack_offset = (Argument::n_int_register_parameters_c+1)* wordSize; // don't overwrite return address
#else
    _num_int_args = (method->is_static() ? 1 : 0);
    _num_fp_args = 0;
    _stack_offset = wordSize; // don't overwrite return address
#endif // _WIN64
#endif // AMD64
  }

  // Code generation
  void generate(uint64_t fingerprint);

  // Code generation support
  static Register from();
  static Register to();
  static Register temp();
};
