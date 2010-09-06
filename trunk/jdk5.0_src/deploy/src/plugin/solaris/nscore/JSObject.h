/*
 * @(#)JSObject.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
