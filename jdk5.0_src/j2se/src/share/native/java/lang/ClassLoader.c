/*
 * @(#)ClassLoader.c	1.74 04/02/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <assert.h>

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "jvm.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_ClassLoader_NativeLibrary.h"

/* defined in libverify.so/verify.dll (src file common/check_format.c) */
extern jboolean VerifyClassname(char *utf_name, jboolean arrayAllowed);
extern jboolean VerifyFixClassname(char *utf_name);

static JNINativeMethod methods[] = {
    {"retrieveDirectives",  "()Ljava/lang/AssertionStatusDirectives;", (void *)&JVM_AssertionStatusDirectives}
};

JNIEXPORT void JNICALL
Java_java_lang_ClassLoader_registerNatives(JNIEnv *env, jclass cls)
{
    (*env)->RegisterNatives(env, cls, methods, 
			    sizeof(methods)/sizeof(JNINativeMethod));
}

/* Convert java string to UTF char*. Use local buffer if possible, 
   otherwise malloc new memory. Returns null IFF malloc failed. */
static char* 
getUTF(JNIEnv *env, jstring str, char* localBuf, int bufSize)
{
    char* utfStr = NULL;

    int len = (*env)->GetStringUTFLength(env, str);
    int unicode_len = (*env)->GetStringLength(env, str);
    if (len >= bufSize) {
	utfStr = malloc(len + 1);
	if (utfStr == NULL) {
	    JNU_ThrowOutOfMemoryError(env, NULL);
	    return NULL;
	}
    } else {
	utfStr = localBuf;
    }
    (*env)->GetStringUTFRegion(env, str, 0, unicode_len, utfStr);

    return utfStr;
}

// The existence or signature of this method is not guaranteed since it
// supports a private method.  This method will be changed in 1.7.
JNIEXPORT jclass JNICALL
Java_java_lang_ClassLoader_defineClass0(JNIEnv *env,
					jobject loader,
					jstring name,
					jbyteArray data,
					jint offset,
					jint length,
					jobject pd)
{
    return Java_java_lang_ClassLoader_defineClass1(env, loader, name, data, offset,
						   length, pd, NULL);
}

JNIEXPORT jclass JNICALL
Java_java_lang_ClassLoader_defineClass1(JNIEnv *env,
					jobject loader,
					jstring name,
					jbyteArray data,
					jint offset,
					jint length,
					jobject pd,
                                        jstring source)
{
    jbyte *body;
    char *utfName;
    jclass result = 0;
    char buf[128];
    char* utfSource;
    char sourceBuf[1024];

    if (data == NULL) {
	JNU_ThrowNullPointerException(env, 0);
	return 0;
    }

    /* Work around 4153825. malloc crashes on Solaris when passed a
     * negative size.
     */
    if (length < 0) {
        JNU_ThrowArrayIndexOutOfBoundsException(env, 0);
	return 0;
    }

    body = (jbyte *)malloc(length);

    if (body == 0) {
        JNU_ThrowOutOfMemoryError(env, 0);
	return 0;
    }

    (*env)->GetByteArrayRegion(env, data, offset, length, body);

    if ((*env)->ExceptionOccurred(env))
        goto free_body;

    if (name != NULL) {
	utfName = getUTF(env, name, buf, sizeof(buf));
	if (utfName == NULL) {
	    JNU_ThrowOutOfMemoryError(env, NULL);
	    goto free_body;
	}
	VerifyFixClassname(utfName);
    } else {
	utfName = NULL;
    }

    if (source != NULL) {
        utfSource = getUTF(env, source, sourceBuf, sizeof(sourceBuf));
	if (utfSource == NULL) {
	    JNU_ThrowOutOfMemoryError(env, NULL);
	    goto free_utfName;
	}
    } else {
	utfSource = NULL;
    }
    result = JVM_DefineClassWithSource(env, utfName, loader, body, length, pd, utfSource); 

    if (utfSource && utfSource != sourceBuf) 
        free(utfSource);

 free_utfName:
    if (utfName && utfName != buf) 
        free(utfName);

 free_body:
    free(body);
    return result;
}

JNIEXPORT jclass JNICALL
Java_java_lang_ClassLoader_defineClass2(JNIEnv *env,
					jobject loader,
					jstring name,
					jobject data,
					jint offset,
					jint length,
					jobject pd,
					jstring source)
{
    jbyte *body;
    char *utfName;
    jclass result = 0;
    char buf[128];
    char* utfSource;
    char sourceBuf[1024];

    assert(data != NULL); // caller fails if data is null.
    assert(length >= 0);  // caller passes ByteBuffer.remaining() for length, so never neg.
    // caller passes ByteBuffer.position() for offset, and capacity() >= position() + remaining()
    assert((*env)->GetDirectBufferCapacity(env, data) >= (offset + length));  

    body = (*env)->GetDirectBufferAddress(env, data);

    if (body == 0) {
        JNU_ThrowNullPointerException(env, 0);
	return 0;
    }

    body += offset;

    if (name != NULL) {
	utfName = getUTF(env, name, buf, sizeof(buf));
	if (utfName == NULL) {
	    JNU_ThrowOutOfMemoryError(env, NULL);
	    return result;
	}
	VerifyFixClassname(utfName);
    } else {
	utfName = NULL;
    }

    if (source != NULL) {
        utfSource = getUTF(env, source, sourceBuf, sizeof(sourceBuf));
	if (utfSource == NULL) {
	    JNU_ThrowOutOfMemoryError(env, NULL);
	    goto free_utfName;
	}
    } else {
	utfSource = NULL;
    }
    result = JVM_DefineClassWithSource(env, utfName, loader, body, length, pd, utfSource); 

    if (utfSource && utfSource != sourceBuf) 
        free(utfSource);

 free_utfName:
    if (utfName && utfName != buf) 
        free(utfName);

    return result;
}

JNIEXPORT void JNICALL
Java_java_lang_ClassLoader_resolveClass0(JNIEnv *env, jobject this,
					 jclass cls)
{
    if (cls == NULL) {
	JNU_ThrowNullPointerException(env, 0);
	return;
    }

    JVM_ResolveClass(env, cls);
}

JNIEXPORT jclass JNICALL
Java_java_lang_ClassLoader_findBootstrapClass(JNIEnv *env, jobject loader, 
					      jstring classname)
{
    char *clname;
    jclass cls = 0;
    char buf[128];

    if (classname == NULL) {
        JNU_ThrowClassNotFoundException(env, 0);
	return 0;
    }

    clname = getUTF(env, classname, buf, sizeof(buf));
    if (clname == NULL) {
	JNU_ThrowOutOfMemoryError(env, NULL);
	return NULL;
    }
    VerifyFixClassname(clname);

    if (!VerifyClassname(clname, JNI_TRUE)) {  /* expects slashed name */
        JNU_ThrowClassNotFoundException(env, clname);
	goto done;
    }

    cls = JVM_FindClassFromClassLoader(env, clname, JNI_FALSE, 0, JNI_FALSE);

 done:
    if (clname != buf) {
    	free(clname);
    }

    return cls;
}

JNIEXPORT jclass JNICALL
Java_java_lang_ClassLoader_findLoadedClass0(JNIEnv *env, jobject loader, 
					   jstring name)
{
    if (name == NULL) {
	return 0;
    } else {
        return JVM_FindLoadedClass(env, loader, name);
    }
}

static jfieldID handleID;
static jfieldID jniVersionID;

static jboolean initIDs(JNIEnv *env)
{
    if (handleID == 0) {
        jclass this = 
	    (*env)->FindClass(env, "java/lang/ClassLoader$NativeLibrary");
	if (this == 0)
	    return JNI_FALSE;
        handleID = (*env)->GetFieldID(env, this, "handle", "J");
	if (handleID == 0)
	    return JNI_FALSE;
	jniVersionID = (*env)->GetFieldID(env, this, "jniVersion", "I");
	if (jniVersionID == 0)
	    return JNI_FALSE;
    }
    return JNI_TRUE;
}

typedef jint (JNICALL *JNI_OnLoad_t)(JavaVM *, void *);
typedef void (JNICALL *JNI_OnUnload_t)(JavaVM *, void *);

/*
 * Class:     java_lang_ClassLoader_NativeLibrary
 * Method:    load
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT void JNICALL 
Java_java_lang_ClassLoader_00024NativeLibrary_load
  (JNIEnv *env, jobject this, jstring name)
{
    const char *cname;
    jint jniVersion;
    void * handle;

    if (!initIDs(env))
        return;

    cname = JNU_GetStringPlatformChars(env, name, 0);
    if (cname == 0)
        return;
    handle = JVM_LoadLibrary(cname);
    if (handle) {
        const char *onLoadSymbols[] = JNI_ONLOAD_SYMBOLS;
        JNI_OnLoad_t JNI_OnLoad;
	int i;
	for (i = 0; i < sizeof(onLoadSymbols) / sizeof(char *); i++) {
	    JNI_OnLoad = (JNI_OnLoad_t) 
	        JVM_FindLibraryEntry(handle, onLoadSymbols[i]);
	    if (JNI_OnLoad) {
	        break;
	    }
	}
	if (JNI_OnLoad) {
	    JavaVM *jvm;
	    (*env)->GetJavaVM(env, &jvm);
	    jniVersion = (*JNI_OnLoad)(jvm, NULL);
	} else {
	    jniVersion = 0x00010001;
	}

 	if ((*env)->ExceptionOccurred(env)) {
	    JNU_ThrowByNameWithLastError(env, "java/lang/UnsatisfiedLinkError",
					 "exception occurred in JNI_OnLoad");
	    JVM_UnloadLibrary(handle);
	    JNU_ReleaseStringPlatformChars(env, name, cname);
	    return;
	}
   
	if (!JVM_IsSupportedJNIVersion(jniVersion)) {
	    char msg[256];
	    jio_snprintf(msg, sizeof(msg),
			 "unsupported JNI version 0x%08X required by %s",
			 jniVersion, cname);
	    JNU_ThrowByName(env, "java/lang/UnsatisfiedLinkError", msg);
	    JVM_UnloadLibrary(handle);
	    JNU_ReleaseStringPlatformChars(env, name, cname);
	    return;
	}
	(*env)->SetIntField(env, this, jniVersionID, jniVersion);
    }
    (*env)->SetLongField(env, this, handleID, ptr_to_jlong(handle));
    JNU_ReleaseStringPlatformChars(env, name, cname);
}

/*
 * Class:     java_lang_ClassLoader_NativeLibrary
 * Method:    unload
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_java_lang_ClassLoader_00024NativeLibrary_unload
  (JNIEnv *env, jobject this)
{
    const char *onUnloadSymbols[] = JNI_ONUNLOAD_SYMBOLS;
    void *handle;
    JNI_OnUnload_t JNI_OnUnload;
    int i;

    if (!initIDs(env))
        return;

    handle = jlong_to_ptr((*env)->GetLongField(env, this, handleID));
    for (i = 0; i < sizeof(onUnloadSymbols) / sizeof(char *); i++) {
        JNI_OnUnload = (JNI_OnUnload_t )
	    JVM_FindLibraryEntry(handle, onUnloadSymbols[i]);
	if (JNI_OnUnload) {
	    break;
	}
    }

    if (JNI_OnUnload) {
        JavaVM *jvm;
	(*env)->GetJavaVM(env, &jvm);
        (*JNI_OnUnload)(jvm, NULL);
    }
    JVM_UnloadLibrary(handle);
}

/*
 * Class:     java_lang_ClassLoader_NativeLibrary
 * Method:    find
 * Signature: (Ljava/lang/String;J)J
 */
JNIEXPORT jlong JNICALL 
Java_java_lang_ClassLoader_00024NativeLibrary_find
  (JNIEnv *env, jobject this, jstring name)
{
    jlong handle;
    const char *cname;
    jlong res;

    if (!initIDs(env))
        return jlong_zero;

    handle = (*env)->GetLongField(env, this, handleID);
    cname = (*env)->GetStringUTFChars(env, name, 0);
    if (cname == 0)
        return jlong_zero;
    res = ptr_to_jlong(JVM_FindLibraryEntry(jlong_to_ptr(handle), cname));
    (*env)->ReleaseStringUTFChars(env, name, cname);
    return res;
}
