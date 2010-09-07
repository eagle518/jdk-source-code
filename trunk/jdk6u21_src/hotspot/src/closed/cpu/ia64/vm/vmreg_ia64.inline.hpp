//
// Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
// ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

inline VMReg RegisterImpl::as_VMReg() {
  if( this==noreg ) return VMRegImpl::Bad();
  return VMRegImpl::as_VMReg(encoding() << 1 );
}

inline VMReg FloatRegisterImpl::as_VMReg() {
  return VMRegImpl::as_VMReg((encoding() << 1) + ConcreteRegisterImpl::max_gpr);
}


inline bool VMRegImpl::is_Register() { return (unsigned int) value() < (unsigned int) ConcreteRegisterImpl::max_gpr; }

inline bool VMRegImpl::is_FloatRegister() { return value() >= ConcreteRegisterImpl::max_gpr &&
                                                   value() < ConcreteRegisterImpl::max_fpr; }
inline Register VMRegImpl::as_Register() {

  assert( is_Register() && is_even(value()), "even-aligned GPR name" );
  // Yuk
  return ::as_Register(value()>>1);
}

inline FloatRegister VMRegImpl::as_FloatRegister() {
  assert( is_FloatRegister(), "must be" );
  // Yuk
  return ::as_FloatRegister( (value() - ConcreteRegisterImpl::max_gpr) >> 1 );
}

inline   bool VMRegImpl::is_concrete() {
  assert(is_reg(), "must be");
  return is_even(value());
}
