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

#include "incls/_precompiled.incl"
#include "incls/_vmreg_x86.cpp.incl"



void VMRegImpl::set_regName() {
  Register reg = ::as_Register(0);
  int i;
  for (i = 0; i < ConcreteRegisterImpl::max_gpr ; ) {
    regName[i++] = reg->name();
#ifdef AMD64
    regName[i++] = reg->name();
#endif // AMD64
    reg = reg->successor();
  }

  FloatRegister freg = ::as_FloatRegister(0);
  for ( ; i < ConcreteRegisterImpl::max_fpr ; ) {
    regName[i++] = freg->name();
    regName[i++] = freg->name();
    freg = freg->successor();
  }

  XMMRegister xreg = ::as_XMMRegister(0);
  for ( ; i < ConcreteRegisterImpl::max_xmm ; ) {
    regName[i++] = xreg->name();
    regName[i++] = xreg->name();
    xreg = xreg->successor();
  }
  for ( ; i < ConcreteRegisterImpl::number_of_registers ; i ++ ) {
    regName[i] = "NON-GPR-FPR-XMM";
  }
}
