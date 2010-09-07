/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodes_ia64.cpp.incl"


void Bytecodes::pd_initialize() {
  // No ia64 specific initialization
}


Bytecodes::Code Bytecodes::pd_base_code_for(Code code) {
  // No ia64 specific bytecodes
  return code;
}
