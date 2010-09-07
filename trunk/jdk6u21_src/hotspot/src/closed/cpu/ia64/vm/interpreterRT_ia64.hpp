/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// native method calls

class SignatureHandlerGenerator: public NativeSignatureIterator {
private:
  MacroAssembler* _masm;

  Argument::Sig _prev_sig;
  int           _prev_jni_offset;
  int           _prev_float_reg_offset;

  void pass_prev(int offset_to_next);
  void pass_curr(Argument::Sig sig);

  void pass_double();
  void pass_float();
  void pass_long();
  void pass_int();
  void pass_object();

public:
  // Creation
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer)
    : NativeSignatureIterator(method),
      _masm(new MacroAssembler(buffer)),
      _prev_sig(Argument::no_sig),
      _prev_float_reg_offset(-1) {
  }

  // Code generation
  void generate(uint64_t fingerprint);
};
