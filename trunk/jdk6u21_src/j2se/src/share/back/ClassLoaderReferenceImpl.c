/*
 * @(#)ClassLoaderReferenceImpl.c	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ClassLoaderReferenceImpl.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
visibleClasses(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env = getEnv();
    jobject loader;
    
    loader = inStream_readClassLoaderRef(env, in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    WITH_LOCAL_REFS(env, 1) {
        
        jvmtiError error;
        jint count;
        jclass *classes;
        int i;
    
        error = allClassLoaderClasses(loader, &classes, &count);
        if (error != JVMTI_ERROR_NONE) {
            outStream_setError(out, map2jdwpError(error));
        } else {
            (void)outStream_writeInt(out, count);
            for (i = 0; i < count; i++) {
                jbyte tag;
                jclass clazz;

                clazz = classes[i];
                tag = referenceTypeTag(clazz);

                (void)outStream_writeByte(out, tag);
                (void)outStream_writeObjectRef(env, out, clazz);
            }
        }

        if ( classes != NULL )
            jvmtiDeallocate(classes);
     
     } END_WITH_LOCAL_REFS(env);

    return JNI_TRUE;
}

void *ClassLoaderReference_Cmds[] = { (void *)0x1
    ,(void *)visibleClasses
};
