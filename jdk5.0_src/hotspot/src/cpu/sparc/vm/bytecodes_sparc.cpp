#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodes_sparc.cpp	1.12 03/12/23 16:36:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodes_sparc.cpp.incl"


void Bytecodes::pd_initialize() {
  // (nothing)
}

Bytecodes::Code Bytecodes::pd_base_code_for(Code code) {
  return code;
}
