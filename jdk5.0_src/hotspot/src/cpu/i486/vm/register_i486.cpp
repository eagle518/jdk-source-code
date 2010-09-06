#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_i486.cpp	1.7 03/12/23 16:36:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_register_i486.cpp.incl"


const char* RegisterImpl::name() const {
  const char* names[number_of_registers] = {
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"
  };
  return is_valid() ? names[encoding()] : "noreg";
}

const char* XMMRegisterImpl::name() const {
  const char* names[number_of_registers] = {
    "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"
  };
  return is_valid() ? names[encoding()] : "xnoreg";
}
