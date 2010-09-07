/*
 * @(#)StringUtil.m	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#import <stdlib.h>
#import "StringUtil.h"
#import "JNIExceptions.h"
 
NSString* jstringToNSString(JNIEnv* env, jstring jstr)
{
    const jchar* jstrChars = env->GetStringChars(jstr, NULL);
    NSString* str = [[NSString alloc] initWithCharacters: jstrChars length: env->GetStringLength(jstr)];
    env->ReleaseStringChars(jstr, jstrChars);
    return str;
}

jstring NSStringToJString(NSString *str, JNIEnv* env)
{
    unichar* strChars = (unichar*) calloc([str length], sizeof(unichar));
    [str getCharacters: strChars];
    jstring jstr = env->NewString(strChars, [str length]);
    free(strChars);
    CLEAR_EXCEPTION(env);
    return jstr;
}
