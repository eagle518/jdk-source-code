/*
 * Copyright (c) 2000, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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
#include "incls/_register_zero.cpp.incl"

const int ConcreteRegisterImpl::max_gpr = RegisterImpl::number_of_registers;
const int ConcreteRegisterImpl::max_fpr =
  ConcreteRegisterImpl::max_gpr + FloatRegisterImpl::number_of_registers;

const char* RegisterImpl::name() const {
  ShouldNotCallThis();
}

const char* FloatRegisterImpl::name() const {
  ShouldNotCallThis();
}
