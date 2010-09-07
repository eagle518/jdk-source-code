/*
 * @(#)ReferenceTypeImpl.c	1.50 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "ReferenceTypeImpl.h"
#include "inStream.h"
#include "outStream.h"


static jboolean 
signature(PacketInputStream *in, PacketOutputStream *out)
{
    char *signature = NULL;    
    jclass clazz;
    jvmtiError error;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = classSignature(clazz, &signature, NULL);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeString(out, signature);
    jvmtiDeallocate(signature);
    
    return JNI_TRUE;
}

static jboolean
signatureWithGeneric(PacketInputStream *in, PacketOutputStream *out)
{
  /* Returns both the signature and the generic signature */
    char *signature = NULL;
    char *genericSignature = NULL;    
    jclass clazz;
    jvmtiError error;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }
    error = classSignature(clazz, &signature, &genericSignature);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeString(out, signature);
    writeGenericSignature(out, genericSignature);
    jvmtiDeallocate(signature);
    if (genericSignature != NULL) {
      jvmtiDeallocate(genericSignature);
    }

    
    return JNI_TRUE;
}

static jboolean 
getClassLoader(PacketInputStream *in, PacketOutputStream *out)
{
    jclass clazz;
    jobject loader;
    jvmtiError error;
 
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = classLoader(clazz, &loader);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeObjectRef(out, loader);
    return JNI_TRUE;
}

static jboolean 
modifiers(PacketInputStream *in, PacketOutputStream *out)
{
    jint modifiers;
    jclass clazz;
    jvmtiError error;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetClassModifiers)
                (gdata->jvmti, clazz, &modifiers);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }
    
    (void)outStream_writeInt(out, modifiers);
    
    return JNI_TRUE;
}

static void 
writeMethodInfo(PacketOutputStream *out, jclass clazz, jmethodID method,
                int outputGenerics)
{
    char *name = NULL;
    char *signature = NULL;
    char *genericSignature = NULL;
    jint modifiers;
    jvmtiError error;
    jboolean isSynthetic;
    
    error = isMethodSynthetic(method, &isSynthetic);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }

    error = methodModifiers(method, &modifiers);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }
    
    error = methodSignature(method, &name, &signature, &genericSignature);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }

    if (isSynthetic) {
        modifiers |= MOD_SYNTHETIC;
    }
    (void)outStream_writeMethodID(out, method);
    (void)outStream_writeString(out, name);
    (void)outStream_writeString(out, signature);
    if (outputGenerics == 1) {
        writeGenericSignature(out, genericSignature);
    }
    (void)outStream_writeInt(out, modifiers);
    jvmtiDeallocate(name);
    jvmtiDeallocate(signature);
    if (genericSignature != NULL) {
      jvmtiDeallocate(genericSignature);
    }
}

static jboolean 
methods1(PacketInputStream *in, PacketOutputStream *out,
         int outputGenerics)
{
    int i;
    jclass clazz;
    jint methodCount = 0;
    jmethodID *methods = NULL;
    jvmtiError error;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetClassMethods)
                (gdata->jvmti, clazz, &methodCount, &methods);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeInt(out, methodCount);
    for (i = 0; (i < methodCount) && !outStream_error(out); i++) {
        writeMethodInfo(out, clazz, methods[i], outputGenerics);
    }

    /* Free methods array */
    if ( methods != NULL ) {
	jvmtiDeallocate(methods);
    }
    return JNI_TRUE;
}

static jboolean 
methods(PacketInputStream *in, PacketOutputStream *out,
         int outputGenerics)
{
    return methods1(in, out, 0);
}

static jboolean
methodsWithGeneric(PacketInputStream *in, PacketOutputStream *out) 
{
    return methods1(in, out, 1);
}


static void
writeFieldInfo(PacketOutputStream *out, jclass clazz, jfieldID fieldID,
               int outputGenerics)
{
    char *name;
    char *signature = NULL;
    char *genericSignature = NULL;
    jint modifiers;
    jboolean isSynthetic;
    jvmtiError error;
    
    error = isFieldSynthetic(clazz, fieldID, &isSynthetic);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }

    error = fieldModifiers(clazz, fieldID, &modifiers);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }
    
    error = fieldSignature(clazz, fieldID, &name, &signature, &genericSignature);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return;
    }
    if (isSynthetic) {
        modifiers |= MOD_SYNTHETIC;
    }
    (void)outStream_writeFieldID(out, fieldID);
    (void)outStream_writeString(out, name);
    (void)outStream_writeString(out, signature);
    if (outputGenerics == 1) {
        writeGenericSignature(out, genericSignature);
    }
    (void)outStream_writeInt(out, modifiers);
    jvmtiDeallocate(name);
    jvmtiDeallocate(signature);
    if (genericSignature != NULL) {
      jvmtiDeallocate(genericSignature);
    }
}

static jboolean 
fields1(PacketInputStream *in, PacketOutputStream *out, int outputGenerics)
{
    int i;
    jclass clazz;
    jint fieldCount = 0;
    jfieldID *fields = NULL;
    jvmtiError error;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetClassFields)
                (gdata->jvmti, clazz, &fieldCount, &fields);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeInt(out, fieldCount);
    for (i = 0; (i < fieldCount) && !outStream_error(out); i++) {
        writeFieldInfo(out, clazz, fields[i], outputGenerics);
    }

    /* Free fields array */
    if ( fields != NULL ) {
	jvmtiDeallocate(fields);
    }
    return JNI_TRUE;
}


static jboolean 
fields(PacketInputStream *in, PacketOutputStream *out)
{
    return fields1(in, out, 0);
}

static jboolean
fieldsWithGeneric(PacketInputStream *in, PacketOutputStream *out) 
{
    return fields1(in, out, 1);

}

static jboolean 
getValues(PacketInputStream *in, PacketOutputStream *out)
{
    sharedGetFieldValues(in, out, JNI_TRUE);
    return JNI_TRUE;
}

static jboolean 
sourceFile(PacketInputStream *in, PacketOutputStream *out)
{
    char *fileName;    
    jvmtiError error;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = JVMTI_FUNC_PTR(gdata->jvmti,GetSourceFileName)
		(gdata->jvmti, clazz, &fileName);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeString(out, fileName);
    jvmtiDeallocate(fileName);
    return JNI_TRUE;
}

static jboolean 
sourceDebugExtension(PacketInputStream *in, PacketOutputStream *out)
{
    char *extension;    
    jvmtiError error;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    error = getSourceDebugExtension(clazz, &extension);
    if (error != JVMTI_ERROR_NONE) {
        outStream_setError(out, map2jdwpError(error));
        return JNI_TRUE;
    }

    (void)outStream_writeString(out, extension);
    jvmtiDeallocate(extension);
    return JNI_TRUE;
}

static jboolean 
nestedTypes(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {
    
	jvmtiError error;
	jint count;
	jclass *nested;
    
	error = allNestedClasses(clazz, &nested, &count);
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    int i;
	    (void)outStream_writeInt(out, count);
	    for (i = 0; i < count; i++) {
		(void)outStream_writeByte(out, referenceTypeTag(nested[i]));
		(void)outStream_writeObjectRef(out, nested[i]);
	    }
	    if ( nested != NULL ) {
		jvmtiDeallocate(nested);
	    }
	}
    
    } END_WITH_LOCAL_REFS(env);

    return JNI_TRUE;
}

static jboolean 
getClassStatus(PacketInputStream *in, PacketOutputStream *out)
{
    jint status;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    status = classStatus(clazz);
    (void)outStream_writeInt(out, map2jdwpClassStatus(status));
    return JNI_TRUE;
}

static jboolean 
interfaces(PacketInputStream *in, PacketOutputStream *out)
{
    JNIEnv *env;
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    env = getEnv();
    
    WITH_LOCAL_REFS(env, 1) {
    
	jvmtiError error;
	jint interfaceCount;
	jclass *interfaces;
    
	error = allInterfaces(clazz, &interfaces, &interfaceCount);
	if (error != JVMTI_ERROR_NONE) {
	    outStream_setError(out, map2jdwpError(error));
	} else {
	    int i;
	    
	    (void)outStream_writeInt(out, interfaceCount);
	    for (i = 0; i < interfaceCount; i++) {
		(void)outStream_writeObjectRef(out, interfaces[i]);
	    }
	    if ( interfaces != NULL ) {
		jvmtiDeallocate(interfaces);
	    }
	}
    
    } END_WITH_LOCAL_REFS(env);

    return JNI_TRUE;
}

static jboolean 
classObject(PacketInputStream *in, PacketOutputStream *out)
{
    jclass clazz;
    
    clazz = inStream_readClassRef(in);
    if (inStream_error(in)) {
        return JNI_TRUE;
    }

    /*
     * In our implementation, the reference type id is the same as the
     * class object id, so we bounce it right back.
     *
     */
    
    (void)outStream_writeObjectRef(out, clazz);
    
    return JNI_TRUE;
}

void *ReferenceType_Cmds[] = { (void *)15
    ,(void *)signature
    ,(void *)getClassLoader
    ,(void *)modifiers
    ,(void *)fields
    ,(void *)methods
    ,(void *)getValues
    ,(void *)sourceFile
    ,(void *)nestedTypes
    ,(void *)getClassStatus
    ,(void *)interfaces
    ,(void *)classObject
    ,(void *)sourceDebugExtension
    ,(void *)signatureWithGeneric
    ,(void *)fieldsWithGeneric
    ,(void *)methodsWithGeneric
};

