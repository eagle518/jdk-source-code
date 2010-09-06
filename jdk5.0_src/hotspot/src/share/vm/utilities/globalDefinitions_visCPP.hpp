#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_visCPP.hpp	1.54 03/12/23 16:44:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds compiler-dependent includes,
// globally used constants & types, class (forward)
// declarations and a few frequently used utility functions.

# include <ctype.h>
# include <string.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stddef.h>// for offsetof
# include <io.h>    // for stream.cpp
# include <float.h> // for _isnan
# include <stdio.h> // for va_list
# include <time.h>
# include <fcntl.h>

// 4810578: varargs unsafe on 32-bit integer/64-bit pointer architectures
// When __cplusplus is defined, NULL is defined as 0 (32-bit constant) in
// system header files.  On 32-bit architectures, there is no problem.
// On 64-bit architectures, defining NULL as a 32-bit constant can cause
// problems with varargs functions: C++ integral promotion rules say for
// varargs, we pass the argument 0 as an int.  So, if NULL was passed to a
// varargs function it will remain 32-bits.  Depending on the calling
// convention of the machine, if the argument is passed on the stack then
// only 32-bits of the "NULL" pointer may be initialized to zero.  The
// other 32-bits will be garbage.  If the varargs function is expecting a
// pointer when it extracts the argument, then we may have a problem.
//
// Solution: For 64-bit architectures, redefine NULL as 64-bit constant 0.
#ifdef _LP64
#undef NULL
// 64-bit Windows uses a P64 data model (not LP64, although we define _LP64)
// Since longs are 32-bit we cannot use 0L here.  Use the Visual C++ specific
// 64-bit integer-suffix (i64) instead.
#define NULL 0i64
#else
#ifndef NULL
#define NULL 0
#endif
#endif

// Compiler-specific primitive types
typedef	unsigned __int8  uint8_t;
typedef	unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#ifdef _WIN64
typedef unsigned __int64 uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif
typedef	signed   __int8  int8_t;
typedef signed   __int16 int16_t;
typedef signed   __int32 int32_t;
typedef signed   __int64 int64_t;
#ifdef _WIN64
typedef signed   __int64 intptr_t;
typedef signed   __int64 ssize_t;
#else
typedef signed   int intptr_t;
typedef signed   int ssize_t;
#endif

//----------------------------------------------------------------------------------------------------
// Additional Java basic types

typedef unsigned char    jubyte;
typedef	unsigned short   jushort;
typedef unsigned int     juint;
typedef unsigned __int64 julong;

//----------------------------------------------------------------------------------------------------
// Special (possibly not-portable) casts
// Cast floats into same-size integers and vice-versa w/o changing bit-pattern

inline jint    jint_cast   (jfloat  x)           { return *(jint*   )&x; }
inline jlong   jlong_cast  (jdouble x)           { return *(jlong*  )&x; }

inline jfloat  jfloat_cast (jint    x)           { return *(jfloat* )&x; }
inline jdouble jdouble_cast(jlong   x)           { return *(jdouble*)&x; }


//----------------------------------------------------------------------------------------------------
// Non-standard stdlib-like stuff:
inline int strcasecmp(const char *s1, const char *s2) { return stricmp(s1,s2); }


//----------------------------------------------------------------------------------------------------
// Debugging

#if _WIN64
extern "C" void breakpoint();
#define BREAKPOINT ::breakpoint()
#else
#define BREAKPOINT __asm { int 3 }
#endif

//----------------------------------------------------------------------------------------------------
// Checking for nanness

inline int g_isnan(jfloat  f)                    { return _isnan(f); }
inline int g_isnan(jdouble f)                    { return _isnan(f); }

//----------------------------------------------------------------------------------------------------
// Checking for finiteness

inline int g_isfinite(jfloat  f)                 { return _finite(f); }
inline int g_isfinite(jdouble f)                 { return _finite(f); }

//----------------------------------------------------------------------------------------------------
// Constant for jlong (specifying an long long constant is C++ compiler specific)

// Build a 64bit integer constant on with Visual C++
#define CONST64(x)  (x ## i64)

const jlong min_jlong = CONST64(0x8000000000000000);
const jlong max_jlong = CONST64(0x7fffffffffffffff);

//----------------------------------------------------------------------------------------------------
// Miscellaneous

inline int vsnprintf(char* buf, size_t count, const char* fmt, va_list argptr) {
  return _vsnprintf(buf, count, fmt, argptr);
}

//------------------------------------------------------------------------------------------------------
// Platform-specific memory areas
// (could potentially be to a more appropriate place - we need some platform-specific constants mechanism)
//
// Size of interpreter. Simply increase if too small (assembler will fail with assertion (in debug mode) if too small)
// On 4/13/98: The size of the Intel interpreter was: 34212 bytes (in debug mode).
// Run with +PrintInterpreter for getting the VM to print out the size.    

// [RGV] Need to check for accurate sizes
#ifdef _LP64
const int Interpreter_Code_Size = 256*1024;
#else
const int Interpreter_Code_Size = 110*1024;
#endif // _LP64


#pragma warning( disable : 4100 ) // unreferenced formal parameter
#pragma warning( disable : 4127 ) // conditional expression is constant
#pragma warning( disable : 4514 ) // unreferenced inline function has been removed
#pragma warning( disable : 4244 ) // possible loss of data
#pragma warning( disable : 4512 ) // assignment operator could not be generated
#pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union (needed in windows.h)
#pragma warning( disable : 4511 ) // copy constructor could not be generated
#pragma warning( disable : 4291 ) // no matching operator delete found; memory will not be freed if initialization thows an exception

// Portability macros
#define PRAGMA_INTERFACE      
#define PRAGMA_IMPLEMENTATION
#define PRAGMA_IMPLEMENTATION_(arg)
#define VALUE_OBJ_CLASS_SPEC    : public _ValueObj

// Formatting.
#define FORMAT64_MODIFIER "I64"
