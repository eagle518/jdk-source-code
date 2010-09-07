/*
 * @(#)ClassTypeImpl.c	1.32 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ClassTypeImpl.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
superclass(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	jclass superclass;
	
	superclass = JNI_FUNC_PTR(env,GetSuperclass)(env,clazz);
	(void)outStream_writeObjectRef(out, superclass);

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jint
readStaticFieldValue(JNIEnv *env, PacketInputStream *in, jclass clazz,
                     jfieldID field, char *signature)
{
    jvalue value;
    jdwpError serror = JDWP_ERROR(NONE);

    switch (signature[0]) {
        case JDWP_TAG(ARRAY):
        case JDWP_TAG(OBJECT):
            value.l = inStream_readObjectRef(in);
            JNI_FUNC_PTR(env,SetStaticObjectField)(env, clazz, field, value.l);
            break;
        
        case JDWP_TAG(BYTE):
            value.b = inStream_readByte(in);
            JNI_FUNC_PTR(env,SetStaticByteField)(env, clazz, field, value.b);
            break;

        case JDWP_TAG(CHAR):
            value.c = inStream_readChar(in);
            JNI_FUNC_PTR(env,SetStaticCharField)(env, clazz, field, value.c);
            break;

        case JDWP_TAG(FLOAT):
            value.f = inStream_readFloat(in);
            JNI_FUNC_PTR(env,SetStaticFloatField)(env, clazz, field, value.f);
            break;

        case JDWP_TAG(DOUBLE):
            value.d = inStream_readDouble(in);
            JNI_FUNC_PTR(env,SetStaticDoubleField)(env, clazz, field, value.d);
            break;

        case JDWP_TAG(INT):
            value.i = inStream_readInt(in);
            JNI_FUNC_PTR(env,SetStaticIntField)(env, clazz, field, value.i);
            break;

        case JDWP_TAG(LONG):
            value.j = inStream_readLong(in);
            JNI_FUNC_PTR(env,SetStaticLongField)(env, clazz, field, value.j);
            break;

        case JDWP_TAG(SHORT):
            value.s = inStream_readShort(in);
            JNI_FUNC_PTR(env,SetStaticShortField)(env, clazz, field, value.s);
            break;

        case JDWP_TAG(BOOLEAN):
            value.z = inStream_readBoolean(in);
            JNI_FUNC_PTR(env,SetStaticBooleanField)(env, clazz, field, value.z);
            break;
    }

    if (JNI_FUNC_PTR(env,ExceptionOccurred)(env)) {
        serror = JDWP_ERROR(INTERNAL);
    }
    
    return serror;
}

static jboolean 
setValues(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jint count;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    count = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, count) {

	int i;
	
	for (i = 0; i < count; i++) {
	    
	    jfieldID field;
	    char *signature = NULL;
	    jvmtiError error;
	    jdwpError serror;
	    
	    field = inStream_readFieldID(in);
            if (inStream_error(in)) {
		break;
            }

	    error = fieldSignature(clazz, field, NULL, &signature, NULL);
	    if (error != JVMTI_ERROR_NONE) {
		break;
	    }
	    
	    serror = readStaticFieldValue(env, in, clazz, field, signature);
	    
	    jvmtiDeallocate(signature);
	    
	    if ( serror != JDWP_ERROR(NONE) ) {
		break;
	    }
	    
	}

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
invokeStatic(PacketInputStream *in, PacketOutputStream *out)
{
    return sharedInvoke(in, out);
}

void *ClassType_Cmds[] = { (void *)0x4
    ,(void *)superclass
    ,(void *)setValues
    ,(void *)invokeStatic
    ,(void *)invokeStatic
};

