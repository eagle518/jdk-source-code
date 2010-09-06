#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_definitions_amd64.cpp	1.3 03/12/23 16:35:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// make sure the defines don't screw up the declarations later on in this file
#define DONT_USE_REGISTER_DEFINES

#include "incls/_precompiled.incl"
#include "incls/_register_definitions_amd64.cpp.incl"

REGISTER_DEFINITION(Register, noreg);
REGISTER_DEFINITION(Register, rax);
REGISTER_DEFINITION(Register, rcx);
REGISTER_DEFINITION(Register, rdx);
REGISTER_DEFINITION(Register, rbx);
REGISTER_DEFINITION(Register, rsp);
REGISTER_DEFINITION(Register, rbp);
REGISTER_DEFINITION(Register, rsi);
REGISTER_DEFINITION(Register, rdi);
REGISTER_DEFINITION(Register, r8);
REGISTER_DEFINITION(Register, r9);
REGISTER_DEFINITION(Register, r10);
REGISTER_DEFINITION(Register, r11);
REGISTER_DEFINITION(Register, r12);
REGISTER_DEFINITION(Register, r13);
REGISTER_DEFINITION(Register, r14);
REGISTER_DEFINITION(Register, r15);

REGISTER_DEFINITION(FloatRegister, xmmnoreg);
REGISTER_DEFINITION(FloatRegister, xmm0);
REGISTER_DEFINITION(FloatRegister, xmm1);
REGISTER_DEFINITION(FloatRegister, xmm2);
REGISTER_DEFINITION(FloatRegister, xmm3);
REGISTER_DEFINITION(FloatRegister, xmm4);
REGISTER_DEFINITION(FloatRegister, xmm5);
REGISTER_DEFINITION(FloatRegister, xmm6);
REGISTER_DEFINITION(FloatRegister, xmm7);
REGISTER_DEFINITION(FloatRegister, xmm8);
REGISTER_DEFINITION(FloatRegister, xmm9);
REGISTER_DEFINITION(FloatRegister, xmm10);
REGISTER_DEFINITION(FloatRegister, xmm11);
REGISTER_DEFINITION(FloatRegister, xmm12);
REGISTER_DEFINITION(FloatRegister, xmm13);
REGISTER_DEFINITION(FloatRegister, xmm14);
REGISTER_DEFINITION(FloatRegister, xmm15);

#ifdef _WIN64

REGISTER_DEFINITION(Register, rarg0);
REGISTER_DEFINITION(Register, rarg1);
REGISTER_DEFINITION(Register, rarg2);
REGISTER_DEFINITION(Register, rarg3);

REGISTER_DEFINITION(FloatRegister, farg0);
REGISTER_DEFINITION(FloatRegister, farg1);
REGISTER_DEFINITION(FloatRegister, farg2);
REGISTER_DEFINITION(FloatRegister, farg3);

#else

REGISTER_DEFINITION(Register, rarg0);
REGISTER_DEFINITION(Register, rarg1);
REGISTER_DEFINITION(Register, rarg2);
REGISTER_DEFINITION(Register, rarg3);
REGISTER_DEFINITION(Register, rarg4);
REGISTER_DEFINITION(Register, rarg5);

REGISTER_DEFINITION(FloatRegister, farg0);
REGISTER_DEFINITION(FloatRegister, farg1);
REGISTER_DEFINITION(FloatRegister, farg2);
REGISTER_DEFINITION(FloatRegister, farg3);
REGISTER_DEFINITION(FloatRegister, farg4);
REGISTER_DEFINITION(FloatRegister, farg5);
REGISTER_DEFINITION(FloatRegister, farg6);
REGISTER_DEFINITION(FloatRegister, farg7);

#endif

REGISTER_DEFINITION(Register, rscratch1);
REGISTER_DEFINITION(Register, rscratch2);

REGISTER_DEFINITION(Register, r15_thread);
