/*
 * Copyright (c) 1997, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#define JNIEXPORT
#define JNIIMPORT
#define JNICALL

typedef int jint;

#ifdef _LP64
  typedef long jlong;
#else
  typedef long long jlong;
#endif

typedef signed char jbyte;
