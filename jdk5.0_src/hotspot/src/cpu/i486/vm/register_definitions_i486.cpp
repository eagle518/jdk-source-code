#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)register_definitions_i486.cpp	1.4 03/12/23 16:36:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// make sure the defines don't screw up the declarations later on in this file
#define DONT_USE_REGISTER_DEFINES

#include "incls/_precompiled.incl"
#include "incls/_register_definitions_i486.cpp.incl"

REGISTER_DEFINITION(Register, noreg);
REGISTER_DEFINITION(Register, eax  );
REGISTER_DEFINITION(Register, ecx  );
REGISTER_DEFINITION(Register, edx  );
REGISTER_DEFINITION(Register, ebx  );
REGISTER_DEFINITION(Register, esp  );
REGISTER_DEFINITION(Register, ebp  );
REGISTER_DEFINITION(Register, esi  );
REGISTER_DEFINITION(Register, edi  );

REGISTER_DEFINITION(XMMRegister, xmm0 );
REGISTER_DEFINITION(XMMRegister, xmm1 );
REGISTER_DEFINITION(XMMRegister, xmm2 );
REGISTER_DEFINITION(XMMRegister, xmm3 );
REGISTER_DEFINITION(XMMRegister, xmm4 );
REGISTER_DEFINITION(XMMRegister, xmm5 );
REGISTER_DEFINITION(XMMRegister, xmm6 );
REGISTER_DEFINITION(XMMRegister, xmm7 );
