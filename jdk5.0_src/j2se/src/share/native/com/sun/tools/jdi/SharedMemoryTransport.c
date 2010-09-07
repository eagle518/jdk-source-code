/*
 * @(#)SharedMemoryTransport.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include <jni.h>
#include "SharedMemory.h"
#include "com_sun_tools_jdi_SharedMemoryTransportService.h"
#include "jdwpTransport.h"
#include "shmemBase.h"
#include "sys.h" 

/*
 * JNI interface to the shared memory transport. These JNI methods 
 * call the base shared memory support to do the real work.
 *
 * That is, this is the front-ends interface to our shared memory
 * transport establishment code.
 */
 
/*
 * When initializing the transport from the front end, we use
 * standard malloc and free for allocation.
 */                                                 
static void *allocateWrapper(jint size) {
    return malloc(size);
}
static jdwpTransportCallback callbacks = {allocateWrapper, free};

void
throwException(JNIEnv *env, char *exceptionClassName, char *message)
{
    jclass excClass = (*env)->FindClass(env, exceptionClassName);
    if ((*env)->ExceptionOccurred(env)) {
        return;
    }
    (*env)->ThrowNew(env, excClass, message);
}

void
throwShmemException(JNIEnv *env, char *message, jint errorCode)
{
    char msg[80];
    char buf[255];

    if (shmemBase_getlasterror(msg, sizeof(msg)) == SYS_OK) {
	sprintf(buf, "%s: %s\n", message, msg);
    } else {
	sprintf(buf, "%s, error code = %d", message, errorCode);      
    }
    throwException(env, "java/io/IOException", buf);
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    accept0
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_accept0
  (JNIEnv *env, jobject thisObject, jlong id, jlong timeout)
{
    SharedMemoryConnection *connection = NULL;
    SharedMemoryTransport *transport = ID_TO_TRANSPORT(id);
    jint rc;

    rc = shmemBase_accept(transport, (long)timeout, &connection);
    if (rc != SYS_OK) {
	if (rc == SYS_TIMEOUT) {
	    throwException(env, "com/sun/jdi/connect/spi/TransportTimeoutException", 
		"Timed out waiting for target VM to connect");
	} else {
	    throwShmemException(env, "shmemBase_accept failed", rc);
	}
	return -1;
    }
    return CONNECTION_TO_ID(connection);
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    attach0
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_attach0
  (JNIEnv *env, jobject thisObject, jstring address, jlong timeout)
{
    SharedMemoryConnection *connection = NULL;
    jint rc;
    const char *addrChars;

    addrChars = (*env)->GetStringUTFChars(env, address, NULL);
    if ((*env)->ExceptionOccurred(env)) {
        return CONNECTION_TO_ID(connection);
    } else if (addrChars == NULL) {
        throwException(env, "java/lang/InternalError", "GetStringUTFChars failed");
        return CONNECTION_TO_ID(connection);
    }

    rc = shmemBase_attach(addrChars, (long)timeout, &connection);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_attach failed", rc);
    } 

    (*env)->ReleaseStringUTFChars(env, address, addrChars);

    return CONNECTION_TO_ID(connection);
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    name
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_name
  (JNIEnv *env, jobject thisObject, jlong id)
{
    char *namePtr;
    jstring nameString = NULL;

    SharedMemoryTransport *transport = ID_TO_TRANSPORT(id);
    jint rc = shmemBase_name(transport, &namePtr);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_name failed", rc);
    } else {
        nameString = (*env)->NewStringUTF(env, namePtr);
        if ((nameString == NULL) && !(*env)->ExceptionOccurred(env)) {
            throwException(env, "java/lang/InternalError", "Unable to create string");
        }
    }
    return nameString;
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_initialize
  (JNIEnv *env, jobject thisObject)
{
    JavaVM *vm;
    jint rc;

    rc = (*env)->GetJavaVM(env, &vm);
    if (rc != 0) {
        throwException(env, "java/lang/InternalError", "Unable to access Java VM");
        return;
    }

    rc = shmemBase_initialize(vm, &callbacks);
    if (rc != SYS_OK) {
        throwException(env, "java/lang/InternalError", "Unable to initialize Shared Memory transport");
        return;
    }
}


/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    startListening0
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_startListening0
  (JNIEnv *env, jobject thisObject, jstring address)
{
    const char *addrChars = NULL;
    jint rc;
    jstring retAddress = NULL;
    SharedMemoryTransport *transport = NULL;


    if (address != NULL) {
        addrChars = (*env)->GetStringUTFChars(env, address, NULL);
        if ((*env)->ExceptionOccurred(env)) {
            return TRANSPORT_TO_ID(transport);
        } else if (addrChars == NULL) {
            throwException(env, "java/lang/InternalError", "GetStringUTFChars failed");
            return TRANSPORT_TO_ID(transport);
        }
    }

    rc = shmemBase_listen(addrChars, &transport);
    if (rc != SYS_OK) {
        throwShmemException(env, "shmemBase_listen failed", rc);
    } 
        
    if (addrChars != NULL) {
        (*env)->ReleaseStringUTFChars(env, address, addrChars);
    }

    return TRANSPORT_TO_ID(transport);
}

/*
 * Class:     com_sun_tools_jdi_SharedMemoryTransport
 * Method:    stopListening0
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sun_tools_jdi_SharedMemoryTransportService_stopListening0
  (JNIEnv *env, jobject thisObject, jlong id)
{
    SharedMemoryTransport *transport = ID_TO_TRANSPORT(id);
    shmemBase_closeTransport(transport);
}

