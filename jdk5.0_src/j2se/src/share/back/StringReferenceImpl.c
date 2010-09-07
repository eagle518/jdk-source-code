/*
 * @(#)StringReferenceImpl.c	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "StringReferenceImpl.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
value(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jstring string;
    
    string = inStream_readStringRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	char *utf;
	
	utf = (char *)JNI_FUNC_PTR(env,GetStringUTFChars)(env, string, NULL);
	(void)outStream_writeString(out, utf);
	JNI_FUNC_PTR(env,ReleaseStringUTFChars)(env, string, utf);

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

void *StringReference_Cmds[] = { (void *)0x1
    ,(void *)value};
