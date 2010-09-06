#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)register_amd64.hpp	1.6 03/12/23 16:35:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Use Register as shortcut
class RegisterImpl;
typedef RegisterImpl* Register;

// The implementation of integer registers for the amd64 architecture

class RegisterImpl :
  public AbstractRegisterImpl 
{
 public:
  enum {
    number_of_registers      = 16,
    number_of_byte_registers = 16
  };

  // construction
  inline friend Register as_Register(int encoding);

  // accessors
  int encoding() const
  { 
    assert(is_valid(), "invalid register"); 
    return (int) this; 
  }

  bool is_valid() const
  {
    return 0 <= (int) this && (int) this < number_of_registers;
  }

  bool has_byte_register() const
  {
    return 0 <= (int) this && (int)this < number_of_byte_registers;
  }

  const char* name() const;
};

inline Register as_Register(int encoding)
{
  return (Register) encoding;
}

// The integer registers of the amd64 architecture

CONSTANT_REGISTER_DECLARATION(Register, noreg, (-1));

CONSTANT_REGISTER_DECLARATION(Register, rax,    (0));
CONSTANT_REGISTER_DECLARATION(Register, rcx,    (1));
CONSTANT_REGISTER_DECLARATION(Register, rdx,    (2));
CONSTANT_REGISTER_DECLARATION(Register, rbx,    (3));
CONSTANT_REGISTER_DECLARATION(Register, rsp,    (4));
CONSTANT_REGISTER_DECLARATION(Register, rbp,    (5));
CONSTANT_REGISTER_DECLARATION(Register, rsi,    (6));
CONSTANT_REGISTER_DECLARATION(Register, rdi,    (7));
CONSTANT_REGISTER_DECLARATION(Register, r8,     (8));
CONSTANT_REGISTER_DECLARATION(Register, r9,     (9));
CONSTANT_REGISTER_DECLARATION(Register, r10,   (10));
CONSTANT_REGISTER_DECLARATION(Register, r11,   (11));
CONSTANT_REGISTER_DECLARATION(Register, r12,   (12));
CONSTANT_REGISTER_DECLARATION(Register, r13,   (13));
CONSTANT_REGISTER_DECLARATION(Register, r14,   (14));
CONSTANT_REGISTER_DECLARATION(Register, r15,   (15));


// Use FloatRegister as shortcut
class FloatRegisterImpl;
typedef FloatRegisterImpl* FloatRegister;

// The implementation of float registers for the amd64 architecture

class FloatRegisterImpl :
  public AbstractRegisterImpl 
{
 public:
  enum {
    number_of_registers = 16
  };

  // construction
  inline friend FloatRegister as_FloatRegister(int encoding);

  // accessors
  int encoding() const
  { 
    assert(is_valid(), "invalid fp register"); 
    return (int) this; 
  }

  bool is_valid() const
  {
    return 0 <= (int) this && (int) this < number_of_registers;
  }

  const char* name() const;
};


inline FloatRegister as_FloatRegister(int encoding)
{
  return (FloatRegister) encoding;
}

// The float registers of the amd64 architecture

CONSTANT_REGISTER_DECLARATION(FloatRegister, xmmnoreg, (-1));

CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm0,      (0));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm1,      (1));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm2,      (2));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm3,      (3));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm4,      (4));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm5,      (5));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm6,      (6));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm7,      (7));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm8,      (8));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm9,      (9));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm10,    (10));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm11,    (11));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm12,    (12));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm13,    (13));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm14,    (14));
CONSTANT_REGISTER_DECLARATION(FloatRegister, xmm15,    (15));


// Need to know the total number of registers of all sorts for SharedInfo.
// Define a class that exports it.
class ConcreteRegisterImpl :
  public AbstractRegisterImpl 
{
 public:
  enum {
    number_of_registers = 
      RegisterImpl::number_of_registers + 
      FloatRegisterImpl::number_of_registers
  };
};
