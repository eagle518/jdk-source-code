/*
 * @(#)Proxy.c	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>

#include "jni.h"
#include "jni_util.h"

#include "java_lang_reflect_Proxy.h"

/* defined in libverify.so/verify.dll (src file common/check_format.c) */
extern jboolean VerifyFixClassname(char *utf_name);

/*
 * Class:     java_lang_reflect_Proxy
 * Method:    defineClass0
 * Signature: (Ljava/lang/ClassLoader;Ljava/lang/String;[BII)Ljava/lang/Class;
 *
 * The implementation of this native static method is a copy of that of
 * the native instance method Java_java_lang_ClassLoader_defineClass0()
 * with the implicit "this" parameter becoming the "loader" parameter.
 */
JNIEXPORT jclass JNICALL
Java_java_lang_reflect_Proxy_defineClass0(JNIEnv *env,
					  jclass ignore,
					  jobject loader,
					  jstring name,
					  jbyteArray data,
					  jint offset,
					  jint length)
{
    jbyte *body;
    char *utfName;
    jclass result = 0;
    char buf[128];

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
        int len = (*env)->GetStringUTFLength(env, name);
	int unicode_len = (*env)->GetStringLength(env, name);
        if (len >= sizeof(buf)) {
            utfName = malloc(len + 1);
            if (utfName == NULL) {
                JNU_ThrowOutOfMemoryError(env, NULL);
                goto free_body;
            }
        } else {
            utfName = buf;
        }
    	(*env)->GetStringUTFRegion(env, name, 0, unicode_len, utfName);
	VerifyFixClassname(utfName);
    } else {
	utfName = NULL;
    }

    result = (*env)->DefineClass(env, utfName, loader, body, length);

    if (utfName && utfName != buf) 
        free(utfName);

 free_body:
    free(body);
    return result;
}
