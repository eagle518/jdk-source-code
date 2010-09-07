/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008 Red Hat, Inc.
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

class SignatureHandler {
 public:
  static SignatureHandler *from_handlerAddr(address handlerAddr) {
    return (SignatureHandler *) handlerAddr;
  }

 public:
  ffi_cif* cif() const {
    return (ffi_cif *) this;
  }

  int argument_count() const {
    return cif()->nargs;
  }

  ffi_type** argument_types() const {
    return (ffi_type**) (cif() + 1);
  }

  ffi_type* argument_type(int i) const {
    return argument_types()[i];
  }

  ffi_type* result_type() const {
    return *(argument_types() + argument_count());
  }

 protected:
  friend class InterpreterRuntime;
  friend class SignatureHandlerLibrary;

  void finalize();
};

class SignatureHandlerGeneratorBase : public NativeSignatureIterator {
 private:
  ffi_cif* _cif;

 protected:
  SignatureHandlerGeneratorBase(methodHandle method, ffi_cif *cif)
    : NativeSignatureIterator(method), _cif(cif) {
    _cif->nargs = 0;
  }

  ffi_cif *cif() const {
    return _cif;
  }

 public:
  void generate(uint64_t fingerprint);

 private:
  void pass_int();
  void pass_long();
  void pass_float();
  void pass_double();
  void pass_object();

 private:
  void push(BasicType type);
  virtual void push(intptr_t value) = 0;
};

class SignatureHandlerGenerator : public SignatureHandlerGeneratorBase {
 private:
  CodeBuffer* _cb;

 public:
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer)
    : SignatureHandlerGeneratorBase(method, (ffi_cif *) buffer->code_end()),
      _cb(buffer) {
    _cb->set_code_end((address) (cif() + 1));
  }

 private:
  void push(intptr_t value) {
    intptr_t *dst = (intptr_t *) _cb->code_end();
    _cb->set_code_end((address) (dst + 1));
    *dst = value;
  }
};

class SlowSignatureHandlerGenerator : public SignatureHandlerGeneratorBase {
 private:
  intptr_t *_dst;

 public:
  SlowSignatureHandlerGenerator(methodHandle method, intptr_t* buf)
    : SignatureHandlerGeneratorBase(method, (ffi_cif *) buf) {
    _dst = (intptr_t *) (cif() + 1);
  }

 private:
  void push(intptr_t value) {
    *(_dst++) = value;
  }

 public:
  SignatureHandler *handler() const {
    return (SignatureHandler *) cif();
  }
};
