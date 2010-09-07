/*
 * Copyright (c) 1998, 2008, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_vmreg.cpp.incl"

// First VMReg value that could refer to a stack slot
VMReg VMRegImpl::stack0 = (VMReg)(intptr_t)((ConcreteRegisterImpl::number_of_registers + 1) & ~1);

// VMRegs are 4 bytes wide on all platforms
const int VMRegImpl::stack_slot_size = 4;
const int VMRegImpl::slots_per_word = wordSize / stack_slot_size;

const int VMRegImpl::register_count = ConcreteRegisterImpl::number_of_registers;
// Register names
const char *VMRegImpl::regName[ConcreteRegisterImpl::number_of_registers];

void VMRegImpl::print_on(outputStream* st) const {
  if( is_reg() ) {
    assert( VMRegImpl::regName[value()], "" );
    st->print("%s",VMRegImpl::regName[value()]);
  } else if (is_stack()) {
    int stk = value() - stack0->value();
    st->print("[%d]", stk*4);
  } else {
    st->print("BAD!");
  }
}
