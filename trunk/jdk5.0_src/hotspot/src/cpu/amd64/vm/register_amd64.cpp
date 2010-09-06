#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_amd64.cpp	1.2 03/12/23 16:35:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_register_amd64.cpp.incl"

const char* RegisterImpl::name() const 
{
  const char* names[number_of_registers] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"
  };
  return is_valid() ? names[encoding()] : "noreg";
}

const char* FloatRegisterImpl::name() const 
{
  const char* names[number_of_registers] = {
    "xmm0",  "xmm1", "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",  "xmm7",
    "xmm8",  "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
  };
  return is_valid() ? names[encoding()] : "xmmnoreg";
}
