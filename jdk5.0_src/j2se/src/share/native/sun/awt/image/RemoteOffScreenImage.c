/*
 * @(#)RemoteOffScreenImage.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
