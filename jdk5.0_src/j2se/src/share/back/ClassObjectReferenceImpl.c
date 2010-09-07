/*
 * @(#)ClassObjectReferenceImpl.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ClassObjectReferenceImpl.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
reflectedType(PacketInputStream *in, PacketOutputStream *out)
{
    jbyte tag;
    jobject object;
    
    object = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    /*
     * In our implementation, the reference type id is the same as the
     * class object id, so we bounce it right back.
     *
     */
    
    tag = referenceTypeTag(object);
    (void)outStream_writeByte(out, tag);
    (void)outStream_writeObjectRef(out, object);
    
    return JNI_TRUE;
}

void *ClassObjectReference_Cmds[] = { (void *)1
    ,(void *)reflectedType
};

