/*
 * Copyright (c) 1997, 2004, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2009 Red Hat, Inc.
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
typedef signed char jbyte;

#ifdef _LP64
typedef long jlong;
#else
typedef long long jlong;
#endif
