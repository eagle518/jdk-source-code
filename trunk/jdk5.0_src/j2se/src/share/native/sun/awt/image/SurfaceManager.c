/*
 * @(#)SurfaceManager.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "sun_awt_image_SurfaceManager.h"

#include "jni.h"

static jfieldID sMgrID;

JNIEXPORT void JNICALL
Java_sun_awt_image_SurfaceManager_initIDs(JNIEnv *env, jclass sm)
{
    jclass bimg = (*env)->FindClass(env, "java/awt/image/BufferedImage");
    if (bimg == NULL) {
        return;
    }

    sMgrID = (*env)->GetFieldID(env, bimg, "surfaceManager",
                                "Lsun/awt/image/SurfaceManager;");
}

JNIEXPORT jobject JNICALL
Java_sun_awt_image_SurfaceManager_getSurfaceManager
    (JNIEnv *env, jclass sm, jobject bufImg)
{
    return (*env)->GetObjectField(env, bufImg, sMgrID);
}

JNIEXPORT void JNICALL
Java_sun_awt_image_SurfaceManager_setSurfaceManager
    (JNIEnv *env, jclass sm, jobject bufImg, jobject sMgr)
{
    (*env)->SetObjectField(env, bufImg, sMgrID, sMgr);
}
