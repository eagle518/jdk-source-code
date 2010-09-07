/*
 * @(#)JSObject.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Native side implemenation to deliver the JSObject call to 
 * browser side (Java ----> Javascript communication through
 * netscape.javascript.JSObject.*
 * 
 *  By X. Lu
 *  
 */

#ifndef __JSOBJECT_H_
#define __JSOBJECT_H_

#include "jni.h"

typedef struct JSMessage_struct {
	int requestID;
    int nativeJSObject;
    int slotindex;
    int utfstr_len;
    char* utfstr;
    int  charstr_len;
    int  charstr_sz;
    jchar* charstr;
    jobjectArray jarr;
    jobject value;
    jobject ctx;
} JSMessage;

void JSHandler(RemoteJNIEnv* env);

#endif
