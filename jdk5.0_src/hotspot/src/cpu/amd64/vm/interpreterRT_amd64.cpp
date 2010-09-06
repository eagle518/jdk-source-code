#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRT_amd64.cpp	1.10 04/01/10 09:40:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRT_amd64.cpp.incl"

#define __ _masm->

// Implementation of SignatureHandlerGenerator

Register InterpreterRuntime::SignatureHandlerGenerator::from() { return r14; }
Register InterpreterRuntime::SignatureHandlerGenerator::to()   { return rsp; }
Register InterpreterRuntime::SignatureHandlerGenerator::temp() { return rscratch1; }

void InterpreterRuntime::SignatureHandlerGenerator::pass_int()
{
  const Address src(from(), -offset() * wordSize);

#ifdef _WIN64
  switch (_num_args) {
  case 0:
    __ movl(rarg1, src);
    _num_args++;
    break;
  case 1:
    __ movl(rarg2, src);
    _num_args++;
    break;
  case 2:
    __ movl(rarg3, src);
    _num_args++;
    break;
  default:
    __ movl(rax, src);
    __ movl(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
    break;
  }
#else
  switch (_num_int_args) {
  case 0:
    __ movl(rarg1, src);
    _num_int_args++;
    break;
  case 1:
    __ movl(rarg2, src);
    _num_int_args++;
    break;
  case 2:
    __ movl(rarg3, src);
    _num_int_args++;
    break;
  case 3:
    __ movl(rarg4, src);
    _num_int_args++;
    break;
  case 4:
    __ movl(rarg5, src);
    _num_int_args++;
    break;
  default:
    __ movl(rax, src);
    __ movl(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
    break;
  }
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_long()
{
  const Address src(from(), -(offset() + 1) * wordSize);

#ifdef _WIN64
  switch (_num_args) {
  case 0:
    __ movq(rarg1, src);
    _num_args++;
    break;
  case 1:
    __ movq(rarg2, src);
    _num_args++;
    break;
  case 2:
    __ movq(rarg3, src);
    _num_args++;
    break;
  case 3:
  default:
    __ movq(rax, src);
    __ movq(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
    break;
  }
#else
  switch (_num_int_args) {
  case 0:
    __ movq(rarg1, src);
    _num_int_args++;
    break;
  case 1:
    __ movq(rarg2, src);
    _num_int_args++;
    break;
  case 2:
    __ movq(rarg3, src);
    _num_int_args++;
    break;
  case 3:
    __ movq(rarg4, src);
    _num_int_args++;
    break;
  case 4:
    __ movq(rarg5, src);
    _num_int_args++;
    break;
  default:
    __ movq(rax, src);
    __ movq(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
    break;
  }
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_float()
{
  const Address src(from(), -offset() * wordSize);

#ifdef _WIN64
  if (_num_args < Argument::n_float_register_parameters-1) {
    __ movss(as_FloatRegister(++_num_args), src);
  } else {
    __ movl(rax, src);
    __ movl(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
  }
#else
  if (_num_fp_args < Argument::n_float_register_parameters) {
    __ movss(as_FloatRegister(_num_fp_args++), src);
  } else {
    __ movl(rax, src);
    __ movl(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
  }
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_double()
{
  const Address src(from(), -(offset() + 1) * wordSize);

#ifdef _WIN64
  if (_num_args < Argument::n_float_register_parameters-1) {
    __ movlpd(as_FloatRegister(++_num_args), src);
  } else {
    __ movq(rax, src);
    __ movq(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
  }
#else
  if (_num_fp_args < 8) {
    __ movlpd(as_FloatRegister(_num_fp_args++), src);
  } else {
    __ movq(rax, src);
    __ movq(Address(to(), _stack_offset), rax);
    _stack_offset += wordSize;
  }
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_object()
{
  const Address src(from(), -offset() * wordSize);

#ifdef _WIN64
  switch (_num_args) {
  case 0:
    assert(offset() == 0, "argument register 1 can only be (non-null) receiver");
    __ leaq(rarg1, src);
    _num_args++;
    break;
  case 1:
    __ leaq(rax, src);
    __ xorl(rarg2, rarg2);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg2, rax);
    _num_args++;
    break;
  case 2:
    __ leaq(rax, src);
    __ xorl(rarg3, rarg3);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg3, rax);
    _num_args++;
    break;
  default:
    __ leaq(rax, src);
    __ xorl(temp(), temp());
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, temp(), rax);
    __ movq(Address(to(), _stack_offset), temp());
    _stack_offset += wordSize;
    break;
  }
#else
  switch (_num_int_args) {
  case 0:
    assert(offset() == 0, "argument register 1 can only be (non-null) receiver");
    __ leaq(rarg1, src);
    _num_int_args++;
    break;
  case 1:
    __ leaq(rax, src);
    __ xorl(rarg2, rarg2);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg2, rax);
    _num_int_args++;
    break;
  case 2:
    __ leaq(rax, src);
    __ xorl(rarg3, rarg3);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg3, rax);
    _num_int_args++;
    break;
  case 3:
    __ leaq(rax, src);
    __ xorl(rarg4, rarg4);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg4, rax);
    _num_int_args++;
    break;
  case 4:
    __ leaq(rax, src);
    __ xorl(rarg5, rarg5);
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, rarg5, rax);
    _num_int_args++;
    break;
  default:
    __ leaq(rax, src);
    __ xorl(temp(), temp());
    __ cmpq(src, 0);
    __ cmovq(Assembler::notEqual, temp(), rax);
    __ movq(Address(to(), _stack_offset), temp());
    _stack_offset += wordSize;
    break;
  }
#endif
}

void InterpreterRuntime::SignatureHandlerGenerator::generate(
  uint64_t fingerprint) 
{
  // generate code to handle arguments
  iterate(fingerprint);

  // return result handler
  __ movq(rax, (int64_t) AbstractInterpreter::
                           result_handler(method()->result_type()));
  __ ret(0);

  __ flush();
}


// Implementation of SignatureHandlerLibrary

void SignatureHandlerLibrary::pd_set_handler(address handler) {}


#ifdef _WIN64
class SlowSignatureHandler 
  : public NativeSignatureIterator 
{
 private:
  intptr_t* _from;
  intptr_t* _to;
  intptr_t* _reg_args;
  intptr_t* _fp_identifiers;
  unsigned int _num_args;

  virtual void pass_int()
  {
    if (_num_args < Argument::n_int_register_parameters-1) {
      *_reg_args++ = *((jint*) _from--);
      _num_args++;
    } else {
      *_to++ = *((jint*) _from--);
    }
  }

  virtual void pass_long()
  { 
    if (_num_args < Argument::n_int_register_parameters-1) {
      *_reg_args++ = *--_from;
      _num_args++;
    } else {
      *_to++ = *--_from;
    }
    _from--;
  }

  virtual void pass_object()
  {
    if (_num_args < Argument::n_int_register_parameters-1) {
      *_reg_args++ = (*_from == 0) ? NULL : (intptr_t) _from;
      _num_args++;
    } else {
      *_to++ = (*_from == 0) ? NULL : (intptr_t) _from;
    }
    _from--; 
  }

  virtual void pass_float()
  {
    if (_num_args < Argument::n_float_register_parameters-1) {
      *_reg_args++ = *((jint*) _from--);
      *_fp_identifiers |= (0x01 << (_num_args*2)); // mark as float
      _num_args++;
    } else {
      *_to++ = *((jint*) _from--);
    }
  }

  virtual void pass_double()
  {
    if (_num_args < Argument::n_float_register_parameters-1) {
      *_reg_args++ = *--_from;
      *_fp_identifiers |= (0x3 << (_num_args*2)); // mark as double
      _num_args++;
    } else {
      *_to++ = *--_from;
    }
    _from--;  
  }

 public:
  SlowSignatureHandler(methodHandle method, intptr_t* from, intptr_t* to) 
    : NativeSignatureIterator(method)
  {
    _from = from;
    _to   = to;

    _reg_args = to - (method->is_static() ? 4 : 5);
    _fp_identifiers = to - 2;
    _to = _to + 4;  // Windows reserves stack space for register arguments
    *(int*) _fp_identifiers = 0;
    _num_args = (method->is_static() ? 1 : 0);
  }
};
#else
class SlowSignatureHandler 
  : public NativeSignatureIterator 
{
 private:
  intptr_t* _from;
  intptr_t* _to;
  intptr_t* _int_args;
  intptr_t* _fp_args;
  intptr_t* _fp_identifiers;
  unsigned int _num_int_args;
  unsigned int _num_fp_args;

  virtual void pass_int()
  {
    if (_num_int_args < Argument::n_int_register_parameters-1) {
      *_int_args++ = *((jint*) _from--);
      _num_int_args++;
    } else {
      *_to++ = *((jint*) _from--);
    }
  }

  virtual void pass_long()
  { 
    if (_num_int_args < Argument::n_int_register_parameters-1) {
      *_int_args++ = *--_from;
      _num_int_args++;
    } else {
      *_to++ = *--_from;
    }
    _from--;
  }

  virtual void pass_object()
  {
    if (_num_int_args < Argument::n_int_register_parameters-1) {
      *_int_args++ = (*_from == 0) ? NULL : (intptr_t) _from;
      _num_int_args++;
    } else {
      *_to++ = (*_from == 0) ? NULL : (intptr_t) _from;
    }
    _from--; 
  }

  virtual void pass_float()
  {
    if (_num_fp_args < Argument::n_float_register_parameters) {
      *_fp_args++ = *((jint*) _from--);
      _num_fp_args++;
    } else {
      *_to++ = *((jint*) _from--);
    }
  }

  virtual void pass_double()
  {
    if (_num_fp_args < Argument::n_float_register_parameters) {
      *_fp_args++ = *--_from;
      *_fp_identifiers |= (1 << _num_fp_args); // mark as double
      _num_fp_args++;
    } else {
      *_to++ = *--_from;
    }
    _from--;  
  }

 public:
  SlowSignatureHandler(methodHandle method, intptr_t* from, intptr_t* to) 
    : NativeSignatureIterator(method)
  {
    _from = from;
    _to   = to;

    _int_args = to - (method->is_static() ? 14 : 15);
    _fp_args =  to - 9;
    _fp_identifiers = to - 10;
    *(int*) _fp_identifiers = 0;
    _num_int_args = (method->is_static() ? 1 : 0);
    _num_fp_args = 0;
  }
};
#endif


IRT_ENTRY(address, 
          InterpreterRuntime::slow_signature_handler(JavaThread* thread,
                                                     methodOop method,
                                                     intptr_t* from,
                                                     intptr_t* to))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");

  // handle arguments
  SlowSignatureHandler(m, 
                       from,
                       to + 1).iterate(CONST64(-1));

  // return result handler
  return AbstractInterpreter::result_handler(m->result_type());
IRT_END
