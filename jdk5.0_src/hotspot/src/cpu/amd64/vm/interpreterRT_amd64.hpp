#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreterRT_amd64.hpp	1.8 04/01/10 09:42:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// native method calls

class SignatureHandlerGenerator
  : public NativeSignatureIterator 
{
 private:
  MacroAssembler* _masm;
#ifdef _WIN64
  unsigned int _num_args;
#else
  unsigned int _num_fp_args;
  unsigned int _num_int_args;
#endif
  int _stack_offset;
  void pass_int();
  void pass_long();
  void pass_float();
  void pass_double();
  void pass_object();

 public:
  // Creation
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer) 
    : NativeSignatureIterator(method) 
  {
    _masm = new MacroAssembler(buffer);
#ifdef _WIN64 
    _num_args = (method->is_static() ? 1 : 0);
    _stack_offset = (Argument::n_int_register_parameters+1)* wordSize; // don't overwrite return address
#else
    _num_int_args = (method->is_static() ? 1 : 0);
    _num_fp_args = 0;
    _stack_offset = wordSize; // don't overwrite return address
#endif
  }

  // Code generation
  void generate(uint64_t fingerprint);

  // Code generation support
  static Register from();
  static Register to();
  static Register temp();
};


static address slow_signature_handler(JavaThread* thread,
                                      methodOop method,
                                      intptr_t* from,
                                      intptr_t* to);


struct IcoResult {
  address osr_code;
  address osr_adapter_frame_address;

  IcoResult(address osr_adapter_frame_address, nmethod *osr_code)
    : osr_adapter_frame_address(osr_adapter_frame_address),
      osr_code((address)osr_code)
    {}

  IcoResult(address osr_code)
    : osr_adapter_frame_address(0),
      osr_code(osr_code)
    {}
};

static inline IcoResult makeIcoResult(address osr_adapter_frame_address, nmethod *osr_code) {
  return IcoResult(osr_adapter_frame_address, osr_code);
}

static inline IcoResult makeIcoResult(address osr_code) {
  return IcoResult(osr_code);
}

