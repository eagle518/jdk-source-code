/*
 * Copyright (c) 1998, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_bytecodes_sparc.cpp.incl"


void Bytecodes::pd_initialize() {
  // (nothing)
}

Bytecodes::Code Bytecodes::pd_base_code_for(Code code) {
  return code;
}
