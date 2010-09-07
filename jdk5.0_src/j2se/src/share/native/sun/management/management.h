/*
 * @(#)management.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>

#include "jni_util.h"
#include "jmm.h"

#ifndef _MANAGEMENT_H_
#define _MANAGEMENT_H_

extern const JmmInterface* jmm_interface;
extern void throw_internal_error(JNIEnv* env, const char* msg);

#endif
