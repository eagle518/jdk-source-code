#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreterRT_i486.hpp	1.19 04/01/10 09:44:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// native method calls

class SignatureHandlerGenerator: public NativeSignatureIterator {
 private:
  MacroAssembler* _masm;

  void move(int from_offset, int to_offset);
  void box(int from_offset, int to_offset);

  void pass_int()                                { move(-offset(), offset() + 1); }
  void pass_long()                               { move(-offset(), offset() + 2); move(-offset() - 1, offset() + 1); }
  void pass_object()                             { box (-offset(), offset() + 1); }

 public:
  // Creation
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer) : NativeSignatureIterator(method) {
    _masm = new MacroAssembler(buffer);
  }

  // Code generation
  void generate(uint64_t fingerprint);

  // Code generation support
  static Register from();
  static Register to();
  static Register temp();
};


static address slow_signature_handler(JavaThread* thread, methodOop method, jint* from, jint* to);


// Return type for InterpreterRuntime::invocation_counter_overflow

typedef jlong IcoResult;

static inline IcoResult makeIcoResult(address osr_adapter_frame_address, nmethod *osr_code)
{ return jlong_from((jint)osr_adapter_frame_address, (jint)osr_code); }

static inline IcoResult makeIcoResult(address osr_code)
{ return (jlong)(osr_code); }
