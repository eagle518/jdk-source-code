/*
 * Copyright (c) 2006, 2007, Oracle and/or its affiliates. All rights reserved.
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

inline VMReg RegisterImpl::as_VMReg() {
  if( this==noreg ) return VMRegImpl::Bad();
#ifdef AMD64
  return VMRegImpl::as_VMReg(encoding() << 1 );
#else
  return VMRegImpl::as_VMReg(encoding() );
#endif // AMD64
}

inline VMReg FloatRegisterImpl::as_VMReg() {
  return VMRegImpl::as_VMReg((encoding() << 1) + ConcreteRegisterImpl::max_gpr);
}

inline VMReg XMMRegisterImpl::as_VMReg() {
  return VMRegImpl::as_VMReg((encoding() << 1) + ConcreteRegisterImpl::max_fpr);
}


inline bool VMRegImpl::is_Register() {
  return (unsigned int) value() < (unsigned int) ConcreteRegisterImpl::max_gpr;
}

inline bool VMRegImpl::is_FloatRegister() {
  return value() >= ConcreteRegisterImpl::max_gpr && value() < ConcreteRegisterImpl::max_fpr;
}

inline bool VMRegImpl::is_XMMRegister() {
  return value() >= ConcreteRegisterImpl::max_fpr && value() < ConcreteRegisterImpl::max_xmm;
}

inline Register VMRegImpl::as_Register() {

  assert( is_Register(), "must be");
  // Yuk
#ifdef AMD64
  return ::as_Register(value() >> 1);
#else
  return ::as_Register(value());
#endif // AMD64
}

inline FloatRegister VMRegImpl::as_FloatRegister() {
  assert( is_FloatRegister() && is_even(value()), "must be" );
  // Yuk
  return ::as_FloatRegister((value() - ConcreteRegisterImpl::max_gpr) >> 1);
}

inline XMMRegister VMRegImpl::as_XMMRegister() {
  assert( is_XMMRegister() && is_even(value()), "must be" );
  // Yuk
  return ::as_XMMRegister((value() - ConcreteRegisterImpl::max_fpr) >> 1);
}

inline   bool VMRegImpl::is_concrete() {
  assert(is_reg(), "must be");
#ifndef AMD64
  if (is_Register()) return true;
#endif // AMD64
  return is_even(value());
}
