#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)register_i486.hpp	1.11 03/12/23 16:36:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Use Register as shortcut
class RegisterImpl;
typedef RegisterImpl* Register;


// The implementation of integer registers for the ia32 architecture

class RegisterImpl: public AbstractRegisterImpl {
 public:
  enum {
    number_of_registers      = 8,
    number_of_byte_registers = 4
  };

  // construction
  friend Register as_Register(int encoding)      { return (Register)encoding; }

  // accessors
  int   encoding() const                         { assert(is_valid(), "invalid register"); return (int)this; }
  bool  is_valid() const                         { return 0 <= (int)this && (int)this < number_of_registers; }
  bool  has_byte_register() const                { return 0 <= (int)this && (int)this < number_of_byte_registers; }
  const char* name() const;
};


// The integer registers of the ia32 architecture

CONSTANT_REGISTER_DECLARATION(Register, noreg, (-1));

CONSTANT_REGISTER_DECLARATION(Register, eax  , ( 0));
CONSTANT_REGISTER_DECLARATION(Register, ecx  , ( 1));
CONSTANT_REGISTER_DECLARATION(Register, edx  , ( 2));
CONSTANT_REGISTER_DECLARATION(Register, ebx  , ( 3));

CONSTANT_REGISTER_DECLARATION(Register, esp  , ( 4));
CONSTANT_REGISTER_DECLARATION(Register, ebp  , ( 5));
CONSTANT_REGISTER_DECLARATION(Register, esi  , ( 6));
CONSTANT_REGISTER_DECLARATION(Register, edi  , ( 7));


// Use FloatRegister as shortcut
class FloatRegisterImpl;
typedef FloatRegisterImpl* FloatRegister;

// The implementation of floating point registers for the ia32 architecture
class FloatRegisterImpl: public AbstractRegisterImpl {
 public:
  enum {
    number_of_registers = 16
  };
};


// Use XMMRegister as shortcut
class XMMRegisterImpl;
typedef XMMRegisterImpl* XMMRegister;

// The implementation of XMM registers for the IA32 architecture
class XMMRegisterImpl: public AbstractRegisterImpl {
 public:
  enum {
    number_of_registers = 8
  };

  // construction
  friend XMMRegister as_XMMRegister(int encoding) { return (XMMRegister)encoding; }

  // accessors
  int   encoding() const                          { assert(is_valid(), "invalid register"); return (int)this; }
  bool  is_valid() const                          { return 0 <= (int)this && (int)this < number_of_registers; }
  const char* name() const;
};


// The XMM registers, for P3 and up chips
CONSTANT_REGISTER_DECLARATION(XMMRegister, xnoreg , (-1));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm0 , ( 0));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm1 , ( 1));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm2 , ( 2));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm3 , ( 3));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm4 , ( 4));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm5 , ( 5));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm6 , ( 6));
CONSTANT_REGISTER_DECLARATION(XMMRegister, xmm7 , ( 7));

// Need to know the total number of registers of all sorts for SharedInfo.
// Define a class that exports it.
class ConcreteRegisterImpl : public AbstractRegisterImpl {
 public:
  enum {
    number_of_registers = RegisterImpl::number_of_registers + FloatRegisterImpl::number_of_registers + XMMRegisterImpl::number_of_registers
  };
};
