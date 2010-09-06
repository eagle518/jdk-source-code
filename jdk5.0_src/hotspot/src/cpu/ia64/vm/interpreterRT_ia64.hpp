#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreterRT_ia64.hpp	1.12 04/01/10 09:46:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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


static address slow_signature_handler(JavaThread* thread, methodOop method, intptr_t* from, intptr_t* to);


#if 0

// Windows compiler ignores the ABI and returns all structures by a hidden reference.

// Return type for InterpreterRuntime::invocation_counter_overflow, such
// that the results are returned in GR_RET and GR_RET1.

// ABI says that 16-byte structs are returned in GR_RET and GR_RET1.

struct IcoResult {
  address osr_adapter_frame_address;
  address osr_code;

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

#else

typedef address IcoResult;

static inline IcoResult makeIcoResult(address osr_adapter_frame_address, nmethod *osr_code)
{ ShouldNotReachHere(); return (IcoResult)0; }

static inline IcoResult makeIcoResult(address osr_code)
{ return (IcoResult)osr_code; }

#endif
