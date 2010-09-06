#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jni_sparc.h	1.9 03/12/23 16:37:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

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
typedef signed char jbyte;
