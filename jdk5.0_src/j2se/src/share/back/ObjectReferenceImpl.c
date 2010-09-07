/*
 * @(#)ObjectReferenceImpl.c	1.31 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ObjectReferenceImpl.h"
#include "commonRef.h"
#include "inStream.h"
#include "outStream.h"

static jboolean 
referenceType(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jobject object;
    
    object = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	jbyte tag;
	jclass clazz;
	
	clazz = JNI_FUNC_PTR(env,GetObjectClass)(env, object);
	tag = referenceTypeTag(clazz);

	(void)outStream_writeByte(out, tag);
	(void)outStream_writeObjectRef(out, clazz);

    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
getValues(PacketInputStream *in, PacketOutputStream *out)
{
    sharedGetFieldValues(in, out, JNI_FALSE);
    return JNI_TRUE;
}


static jvmtiError
readFieldValue(JNIEnv *env, PacketInputStream *in, jclass clazz,
               jobject object, jfieldID field, char *signature)
{
    jvalue value;
    jvmtiError error;

    switch (signature[0]) {
        case JDWP_TAG(ARRAY):
        case JDWP_TAG(OBJECT):
            value.l = inStream_readObjectRef(in);
            JNI_FUNC_PTR(env,SetObjectField)(env, object, field, value.l);
            break;
        
        case JDWP_TAG(BYTE):
            value.b = inStream_readByte(in);
            JNI_FUNC_PTR(env,SetByteField)(env, object, field, value.b);
            break;

        case JDWP_TAG(CHAR):
            value.c = inStream_readChar(in);
            JNI_FUNC_PTR(env,SetCharField)(env, object, field, value.c);
            break;

        case JDWP_TAG(FLOAT):
            value.f = inStream_readFloat(in);
            JNI_FUNC_PTR(env,SetFloatField)(env, object, field, value.f);
            break;

        case JDWP_TAG(DOUBLE):
            value.d = inStream_readDouble(in);
            JNI_FUNC_PTR(env,SetDoubleField)(env, object, field, value.d);
            break;

        case JDWP_TAG(INT):
            value.i = inStream_readInt(in);
            JNI_FUNC_PTR(env,SetIntField)(env, object, field, value.i);
            break;

        case JDWP_TAG(LONG):
            value.j = inStream_readLong(in);
            JNI_FUNC_PTR(env,SetLongField)(env, object, field, value.j);
            break;

        case JDWP_TAG(SHORT):
            value.s = inStream_readShort(in);
            JNI_FUNC_PTR(env,SetShortField)(env, object, field, value.s);
            break;

        case JDWP_TAG(BOOLEAN):
            value.z = inStream_readBoolean(in);
            JNI_FUNC_PTR(env,SetBooleanField)(env, object, field, value.z);
            break;
    }

    error = JVMTI_ERROR_NONE;
    if (JNI_FUNC_PTR(env,ExceptionOccurred)(env)) {
        error = JVMTI_ERROR_INTERNAL;
    }
    
    return error;
}

static jboolean 
setValues(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jint count;
    jvmtiError error;
    jobject object;
    
    object = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    count = inStream_readInt(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    error = JVMTI_ERROR_NONE;
    
    WITH_LOCAL_REFS(env, count + 1) {

        jclass clazz;
	
	clazz = JNI_FUNC_PTR(env,GetObjectClass)(env, object);
	
	if (clazz != NULL ) {
	
	    int i;
	    
	    for (i = 0; (i < count) && !inStream_error(in); i++) {
		
		jfieldID field;
		char *signature = NULL;
		
		field = inStream_readFieldID(in);
		if (inStream_error(in))
		    break;

		error = fieldSignature(clazz, field, NULL, &signature, NULL);
		if (error != JVMTI_ERROR_NONE) {
		    break;
	        }

		error = readFieldValue(env, in, clazz, object, field, signature);
		jvmtiDeallocate(signature);
		
		if (error != JVMTI_ERROR_NONE) {
		    break;
	        }
	    }
	}

	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	}
    
    } END_WITH_LOCAL_REFS(env);

    return JNI_TRUE;
}

static jboolean 
monitorInfo(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jobject object;
    
    object = inStream_readObjectRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {

	jvmtiError error;
	jvmtiMonitorUsage info;
	
	(void)memset(&info, 0, sizeof(info));
	error = JVMTI_FUNC_PTR(gdata->jvmti,GetObjectMonitorUsage)
			(gdata->jvmti, object, &info);
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    int i;
	    (void)outStream_writeObjectRef(out, info.owner);
	    (void)outStream_writeInt(out, info.entry_count);
	    (void)outStream_writeInt(out, info.waiter_count);
	    for (i = 0; i < info.waiter_count; i++) {
		(void)outStream_writeObjectRef(out, info.waiters[i]);
	    }
	}
    
	if (info.waiters != NULL )
	    jvmtiDeallocate(info.waiters);
	
    } END_WITH_LOCAL_REFS(env);
    
    return JNI_TRUE;
}

static jboolean 
invokeInstance(PacketInputStream *in, PacketOutputStream *out)
{
    return sharedInvoke(in, out);
}

static jboolean 
disableCollection(PacketInputStream *in, PacketOutputStream *out)
{
    jlong id;
    jvmtiError error;
    
    id = inStream_readObjectID(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = commonRef_pin(id);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }

    return JNI_TRUE;
}

static jboolean 
enableCollection(PacketInputStream *in, PacketOutputStream *out)
{
    jvmtiError error;
    jlong id;
    
    id = inStream_readObjectID(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = commonRef_unpin(id);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
    }

    return JNI_TRUE;
}

static jboolean 
isCollected(PacketInputStream *in, PacketOutputStream *out)
{
    jobject ref;
    jlong id;

    id = inStream_readObjectID(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    if (id == NULL_OBJECT_ID) {
        outStream_setError(out, JDWP_ERROR(INVALID_OBJECT));
        return JNI_TRUE;
    }

    ref = commonRef_idToRef(id);
    (void)outStream_writeBoolean(out, (jboolean)(ref == NULL));

    commonRef_idToRef_delete(NULL, ref);

    return JNI_TRUE;
}

void *ObjectReference_Cmds[] = { (void *)0x9
    ,(void *)referenceType
    ,(void *)getValues
    ,(void *)setValues
    ,(void *)NULL      /* no longer used */
    ,(void *)monitorInfo
    ,(void *)invokeInstance
    ,(void *)disableCollection
    ,(void *)enableCollection
    ,(void *)isCollected
    };
