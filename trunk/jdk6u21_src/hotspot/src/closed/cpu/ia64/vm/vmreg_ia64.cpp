//
// Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
// ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

#include "incls/_precompiled.incl"
#include "incls/_vmreg_ia64.cpp.incl"


void VMRegImpl::set_regName() {
  Register reg = ::as_Register(0);
  int i;
  for (i = 0; i < ConcreteRegisterImpl::max_gpr ; ) {
    regName[i++  ] = reg->name();
    regName[i++  ] = reg->name();
    reg = reg->successor();
  }

  FloatRegister freg = ::as_FloatRegister(0);
  for ( ; i < ConcreteRegisterImpl::max_fpr ; ) {
    regName[i++] = freg->name();
    regName[i++] = freg->name();
    freg = freg->successor();
  }

  for ( ; i < ConcreteRegisterImpl::number_of_registers ; i++ ) {
    regName[i] = "NON-GPR-FPR";
  }

}
