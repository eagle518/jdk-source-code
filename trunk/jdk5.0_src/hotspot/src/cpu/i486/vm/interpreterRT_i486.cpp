#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRT_i486.cpp	1.42 04/01/10 09:43:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRT_i486.cpp.incl"


#define __ _masm->


// Implementation of SignatureHandlerGenerator

void InterpreterRuntime::SignatureHandlerGenerator::move(int from_offset, int to_offset) {
  __ movl(temp(), Address(from(), from_offset * wordSize));
  __ movl(Address(to(), to_offset * wordSize), temp());
}


void InterpreterRuntime::SignatureHandlerGenerator::box(int from_offset, int to_offset) {
  __ leal(temp(), Address(from(), from_offset * wordSize));
  __ cmpl(Address(from(), from_offset * wordSize), 0); // do not use temp() to avoid AGI
  Label L;
  __ jcc(Assembler::notZero, L);
  __ movl(temp(), 0);
  __ bind(L);
  __ movl(Address(to(), to_offset * wordSize), temp());
}


void InterpreterRuntime::SignatureHandlerGenerator::generate( uint64_t fingerprint) {
  // generate code to handle arguments
  iterate(fingerprint);
  // return result handler
  __ movl(eax, (int)AbstractInterpreter::result_handler(method()->result_type()));
  // return
  __ ret(0);
  __ flush();
}


Register InterpreterRuntime::SignatureHandlerGenerator::from()       { return edi; }
Register InterpreterRuntime::SignatureHandlerGenerator::to()         { return esp; }
Register InterpreterRuntime::SignatureHandlerGenerator::temp()       { return ecx; }


// Implementation of SignatureHandlerLibrary

void SignatureHandlerLibrary::pd_set_handler(address handler) {}


class SlowSignatureHandler: public NativeSignatureIterator {
 private:
  jint* _from;
  jint* _to;

  virtual void pass_int()                        { *_to++ = *_from--; }
  virtual void pass_long()                       { _to[0] = _from[-1]; _to[1] = _from[0]; _to += 2; _from -= 2; }
  virtual void pass_object()                     { *_to++ = (*_from == 0) ? 0 : (jint)_from; _from--; }

 public:
  SlowSignatureHandler(methodHandle method, jint* from, jint* to) : NativeSignatureIterator(method) {
    _from = from;
    _to   = to;
  }
};


IRT_ENTRY(address, InterpreterRuntime::slow_signature_handler(JavaThread* thread, methodOop method, jint* from, jint* to))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // handle arguments
  SlowSignatureHandler(m, from, to + 1).iterate(CONST64(-1));
  // return result handler
  return AbstractInterpreter::result_handler(m->result_type());
IRT_END



