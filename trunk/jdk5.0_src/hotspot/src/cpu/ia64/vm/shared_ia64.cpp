#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)shared_ia64.cpp	1.5 03/12/23 16:36:50 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_shared_ia64.cpp.incl"



void SharedInfo::set_regName() {
  Register reg = GR0;
  for (uint i = 0;
       i < RegisterImpl::number_of_registers;
       ++i, reg = reg->successor()) {
    regName[i] = reg->name();
  }

#ifdef COMPILER1
  uint fbase = RegisterImpl::number_of_registers;
  FloatRegister freg = FR0;
  for (uint i = 0;
       i < FloatRegisterImpl::number_of_registers;
       ++i, freg = freg->successor()) {
    regName[fbase+i] = freg->name();
  }

  uint pbase = RegisterImpl::number_of_registers +
               FloatRegisterImpl::number_of_registers;
  PredicateRegister preg = PR0;
  for (uint i = 0;
       i < PredicateRegisterImpl::number_of_registers;
       ++i, preg = preg->successor()) {
    regName[pbase+i] = preg->name();
  }

  uint bbase = RegisterImpl::number_of_registers +
               FloatRegisterImpl::number_of_registers +
               PredicateRegisterImpl::number_of_registers;
  BranchRegister breg = BR0;
  for (uint i = 0;
       i < BranchRegisterImpl::number_of_registers;
       ++i, breg = breg->successor()) {
    regName[bbase+i] = breg->name();
  }

  uint abase = RegisterImpl::number_of_registers +
               FloatRegisterImpl::number_of_registers +
               PredicateRegisterImpl::number_of_registers +
               BranchRegisterImpl::number_of_registers;
  ApplicationRegister areg = AR0;
  for (uint i = 0;
       i < ApplicationRegisterImpl::number_of_registers;
       ++i, areg = areg->successor()) {
    regName[abase+i] = areg->name();
  }
#endif
}
