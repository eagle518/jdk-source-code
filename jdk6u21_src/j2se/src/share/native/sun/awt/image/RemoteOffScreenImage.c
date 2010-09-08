/*
 * @(#)RemoteOffScreenImage.c	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_image_RemoteOffScreenImage.h"

#include "jni.h"

static jfieldID rasterID;

JNIEXPORT void JNICALL
Java_sun_awt_image_RemoteOffScreenImage_initIDs(JNIEnv *env, jclass bisd)
{
    jclass bimg = (*env)->FindClass(env, "java/awt/image/BufferedImage");
    if (bimg == NULL) {
	return;
    }

    rasterID = (*env)->GetFieldID(env, bimg, "raster",
                                  "Ljava/awt/image/WritableRaster;");
}

JNIEXPORT void JNICALL
Java_sun_awt_image_RemoteOffScreenImage_setRasterNative
    (JNIEnv *env, jobject bufImg, jobject raster)
{
    (*env)->SetObjectField(env, bufImg, rasterID, raster);
}
