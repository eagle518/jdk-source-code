#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRT_sparc.cpp	1.62 04/01/10 09:47:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRT_sparc.cpp.incl"


#define __ _masm->


// Implementation of SignatureHandlerGenerator

void InterpreterRuntime::SignatureHandlerGenerator::pass_word(int size_of_arg, int offset_in_arg) {
  Argument  jni_arg(jni_offset() + offset_in_arg, false);
  Register     Rtmp = O0;
  __ ld(Llocals, -offset() * wordSize, Rtmp);

  __ store_argument(Rtmp, jni_arg);
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_long() {
  Argument  jni_arg(jni_offset(), false);
  Register  Rtmp = O0;
  Register  Rtmp1 = G3_scratch;

#ifdef _LP64
  __ ldx(Llocals, -(offset() + 1) * wordSize, Rtmp);
  __ store_long_argument(Rtmp, jni_arg);
#else
  __ ld(Llocals, -(offset() + 1) * wordSize, Rtmp);
  __ store_argument(Rtmp, jni_arg            );
  __ ld(Llocals, -(offset() + 0) * wordSize, Rtmp);
  __ store_argument(Rtmp, jni_arg.successor());
#endif
}


#ifdef _LP64
void InterpreterRuntime::SignatureHandlerGenerator::pass_float() {
  Argument  jni_arg(jni_offset(), false);
  FloatRegister  Rtmp = F0;
  __ ldf(FloatRegisterImpl::S, Llocals, -offset() * wordSize, Rtmp);
  __ store_float_argument(Rtmp, jni_arg);
}
#endif


void InterpreterRuntime::SignatureHandlerGenerator::pass_double() {
  Argument  jni_arg(jni_offset(), false);
#ifdef _LP64
  FloatRegister  Rtmp = F0;
  __ ldf(FloatRegisterImpl::D, Llocals, -(offset() + 1) * wordSize, Rtmp);
  __ store_double_argument(Rtmp, jni_arg);
#else
  Register  Rtmp = O0;
  __ ld(Llocals, -(offset() + 1) * wordSize, Rtmp);
  __ store_argument(Rtmp, jni_arg);
  __ ld(Llocals, -offset() * wordSize, Rtmp);
  __ store_argument(Rtmp, jni_arg.successor());
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_object() {
  Argument  jni_arg(jni_offset(), false);
  Argument java_arg(    offset(), true);
  Register    Rtmp1 = O0;
  Register    Rtmp2 =  jni_arg.is_register() ?  jni_arg.as_register() : O0;


  // the handle for a receiver will never be null
  bool do_NULL_check = offset() != 0 || is_static();

  Address     h_arg = Address(Llocals, 0, -offset() * wordSize);
  __ ld_ptr(h_arg, Rtmp1);
  if (!do_NULL_check) {
    __ add(h_arg, Rtmp2);
  } else {
    if (Rtmp1 == Rtmp2)
          __ tst(Rtmp1);
    else  __ addcc(G0, Rtmp1, Rtmp2); // optimize mov/test pair
    Label L;
    __ brx(Assembler::notZero, true, Assembler::pt, L);
    __ delayed()->add(h_arg, Rtmp2);
    __ bind(L);
  }
  __ store_ptr_argument(Rtmp2, jni_arg);    // this is often a no-op
}


void InterpreterRuntime::SignatureHandlerGenerator::generate( uint64_t fingerprint) {

  // generate code to handle arguments
  iterate(fingerprint);

  // return result handler
  Address result_handler(Lscratch, AbstractInterpreter::result_handler(method()->result_type()));
  __ sethi(result_handler);
  __ retl();
  __ delayed()->add(result_handler, result_handler.base());

  __ flush();
}


// Implementation of SignatureHandlerLibrary

void SignatureHandlerLibrary::pd_set_handler(address handler) {}


class SlowSignatureHandler: public NativeSignatureIterator {
 private:
  intptr_t* _from;
  intptr_t* _to;
  intptr_t  *_RegArgSignature;			// Signature of first Arguments to be passed in Registers
  int _argcount;

  enum {					// We need to differenciate float from non floats in reg args
	non_float = 0,
	float_sig = 1,
	double_sig = 2,
	long_sig = 3
  };

  virtual void pass_int()                        { *_to++ = *((jint *)_from); _from--; 
						   add_signature( non_float ); }
#ifdef _LP64
  virtual void pass_long()                       { _to[0] = _from[-1]; _to += 1; _from -= 2;
                                                   add_signature( long_sig ); }
#else
  virtual void pass_long()                       { _to[0] = _from[-1]; _to[1] = _from[0]; _to += 2; _from -= 2; 
						   add_signature( non_float ); }
#endif

  virtual void pass_object()                     { *_to++ = (*_from == 0) ? NULL : (intptr_t)_from; _from--; 
						   add_signature( non_float ); }

#ifdef _LP64
  virtual void pass_float()                      { *_to++ = *((jint *)_from); _from--; 
						   add_signature( float_sig ); }
#endif

#ifdef _LP64
  virtual void pass_double()                     { *_to++ = *--_from; _from--;  
                                                   add_signature( double_sig ); }
#else
  virtual void pass_double()                     { _to[0] = _from[-1]; _to[1] = _from[0]; _to += 2; _from -= 2; 
                                                   add_signature( double_sig ); }
#endif

  virtual void add_signature( intptr_t sig_type ) { 
    if ( _argcount < (sizeof (intptr_t))*4 ) { 
      *_RegArgSignature |= (sig_type << (_argcount*2) );
      _argcount++;
    }
  }
						   
  
 public:
  SlowSignatureHandler(methodHandle method, intptr_t* from, intptr_t* to, intptr_t *RegArgSig) : NativeSignatureIterator(method) {
    _from = from;
    _to   = to;
    _RegArgSignature = RegArgSig;	
    *_RegArgSignature = 0;
    _argcount = method->is_static() ? 2 : 1;
  }
};


IRT_ENTRY(address, InterpreterRuntime::slow_signature_handler(JavaThread* thread, methodOop method, intptr_t* from, intptr_t* to ))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // handle arguments
  // Warning: We use reg arg slot 00 temporarily to return the RegArgSignature
  // back to the code that pops the arguments into the CPU registers
  SlowSignatureHandler(m, from, m->is_static() ? to+2 : to+1, to).iterate(CONST64(-1));
  // return result handler
  return AbstractInterpreter::result_handler(m->result_type());
IRT_END
