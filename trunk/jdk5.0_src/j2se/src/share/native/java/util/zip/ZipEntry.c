/*
 * @(#)ZipEntry.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Native method support for java.util.zip.ZipEntry
 */

#include <stdio.h>
#include <stdlib.h>
#include "jlong.h"
#include "jvm.h"
#include "jni.h"
#include "jni_util.h"
#include "zip_util.h"

#include "java_util_zip_ZipEntry.h"

#define DEFLATED 8
#define STORED 0

static jfieldID nameID;
static jfieldID timeID;
static jfieldID crcID;
static jfieldID sizeID;
static jfieldID csizeID;
static jfieldID methodID;
static jfieldID extraID;
static jfieldID commentID;

JNIEXPORT void JNICALL
Java_java_util_zip_ZipEntry_initIDs(JNIEnv *env, jclass cls)
{
    nameID = (*env)->GetFieldID(env, cls, "name", "Ljava/lang/String;");
    timeID = (*env)->GetFieldID(env, cls, "time", "J");
    crcID = (*env)->GetFieldID(env, cls, "crc", "J");
    sizeID = (*env)->GetFieldID(env, cls, "size", "J");
    csizeID = (*env)->GetFieldID(env, cls, "csize", "J");
    methodID = (*env)->GetFieldID(env, cls, "method", "I");
    extraID = (*env)->GetFieldID(env, cls, "extra", "[B");
    commentID = (*env)->GetFieldID(env, cls, "comment", "Ljava/lang/String;");
}

JNIEXPORT void JNICALL
Java_java_util_zip_ZipEntry_initFields(JNIEnv *env, jobject obj, jlong zentry)
{
    jzentry *ze = jlong_to_ptr(zentry);
    jstring name = (*env)->GetObjectField(env, obj, nameID);

    if (name == 0) {
        name = (*env)->NewStringUTF(env, ze->name);
	if (name == 0) {
	    return;
	}
	(*env)->SetObjectField(env, obj, nameID, name);
    }
    (*env)->SetLongField(env, obj, timeID, (jlong)ze->time & 0xffffffffUL);
    (*env)->SetLongField(env, obj, crcID, (jlong)ze->crc & 0xffffffffUL);
    (*env)->SetLongField(env, obj, sizeID, (jlong)ze->size);
    if (ze->csize == 0) {
	(*env)->SetLongField(env, obj, csizeID, (jlong)ze->size);
	(*env)->SetIntField(env, obj, methodID, STORED);
    } else {
	(*env)->SetLongField(env, obj, csizeID, (jlong)ze->csize);
	(*env)->SetIntField(env, obj, methodID, DEFLATED);
    }
    if (ze->extra != 0) {
	unsigned char *bp = (unsigned char *)&ze->extra[0];
	jsize len = (bp[0] | (bp[1] << 8));
	jbyteArray extra = (*env)->NewByteArray(env, len);
	if (extra == 0) {
	    return;
	}
	(*env)->SetByteArrayRegion(env, extra, 0, len, &ze->extra[2]);
	(*env)->SetObjectField(env, obj, extraID, extra);
    }
    if (ze->comment != 0) {
	jstring comment = (*env)->NewStringUTF(env, ze->comment);
	if (comment == 0) {
	    return;
	}
	(*env)->SetObjectField(env, obj, commentID, comment);
    }
}
