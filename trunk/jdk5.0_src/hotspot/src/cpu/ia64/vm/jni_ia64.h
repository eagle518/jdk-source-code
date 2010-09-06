#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jni_ia64.h	1.4 03/12/23 16:36:45 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
// HotSpot integration note:
//
// This file and win32 jni_md.h used with the classic VM are identical,
// apart from the #ifdef SOLARIS part.
// Matches jni_md.h 1.9 98/09/21 from JDK1.3 beta H build.
*/

#ifndef _JAVASOFT_JNI_MD_H_
#define _JAVASOFT_JNI_MD_H_

#if defined(SOLARIS) || defined(LINUX)
#ifdef  __cplusplus
  #define JNIEXPORT extern "C"
  #define JNIIMPORT extern "C"
#else
  #define JNIEXPORT
  #define JNIIMPORT
#endif
  #define JNICALL

  typedef int jint;
  typedef long long jlong;
#else
  #define JNIEXPORT __declspec(dllexport)
  #define JNIIMPORT __declspec(dllimport)
  #define JNICALL __stdcall

  typedef int jint;
  typedef __int64 jlong;
#endif

typedef signed char jbyte;

#endif /* !_JAVASOFT_JNI_MD_H_ */

