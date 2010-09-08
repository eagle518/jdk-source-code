/*
 * @(#)management.h	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>

#include "jni_util.h"
#include "jmm.h"

#ifndef _MANAGEMENT_H_
#define _MANAGEMENT_H_

extern const JmmInterface* jmm_interface;
extern jint jmm_version;
extern void throw_internal_error(JNIEnv* env, const char* msg);

#endif
