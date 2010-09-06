#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodes_ia64.cpp	1.4 03/12/23 16:36:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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

