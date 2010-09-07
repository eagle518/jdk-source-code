/*
 * @(#)StringUtil.h	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#import <Foundation/NSString.h>
#include <jni.h>

NSString* jstringToNSString(JNIEnv* env, jstring jstr);
jstring NSStringToJString(NSString *str, JNIEnv* env);
