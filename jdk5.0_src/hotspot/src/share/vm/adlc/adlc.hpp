#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)adlc.hpp	1.17 03/12/23 16:38:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Standard include file for ADLC parser
//

// standard library constants
#include "stdio.h"
#include <iostream.h>
#include "stdlib.h"
#include "string.h"
#include "ctype.h"
#include "stdarg.h"
#include <sys/types.h>

//using namespace std;

// Booleans: TODO-FIXME: bool should probaby be an "int" for LP64 systems
#define bool    int			

/* Make sure that we have the intptr_t and uintptr_t definitions */
#ifdef _WIN32
#ifndef _INTPTR_T_DEFINED
#ifdef _WIN64
typedef __int64 intptr_t;
#else
typedef int intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef _WIN64
typedef unsigned __int64 uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif
#endif // _WIN32

#ifdef LINUX
  #include <inttypes.h>
#endif // LINUX

#define true    v_true
#define false   v_false

const bool true        = 1;
const bool false       = 0;

// Macros 
#define uint32 unsigned int
#define uint   unsigned int

// Macros
// Debugging note:  Put a breakpoint on "abort".
#define assert(cond, msg) { if (!(cond)) { fprintf(stderr, "assert fails %s %d: %s\n", __FILE__, __LINE__, msg); abort(); }}
#define max(a, b)   (((a)>(b)) ? (a) : (b))

// VM components
#include "opcodes.hpp" 

// ADLC components
#include "arena.hpp"
#include "adlcVMDeps.hpp"
#include "filebuff.hpp"
#include "dict2.hpp"
#include "forms.hpp"
#include "formsopt.hpp"
#include "formssel.hpp"
#include "archDesc.hpp"
#include "adlparse.hpp"

