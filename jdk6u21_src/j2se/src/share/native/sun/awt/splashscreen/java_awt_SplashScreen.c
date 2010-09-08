/*
* @(#)java_awt_SplashScreen.c	1.5 10/03/23
*
* Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
* ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
*/

#include "splashscreen_impl.h"
#include <jni.h>
#include <jlong_md.h>

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM * vm, void *reserved)
{
    return JNI_VERSION_1_2;
}

/* FIXME: safe_ExceptionOccured, why and how? */

/*
* Class:     java_awt_SplashScreen
* Method:    _update
* Signature: (J[IIIIII)V
*/
JNIEXPORT void JNICALL
Java_java_awt_SplashScreen__1update(JNIEnv * env, jclass thisClass, 
                                    jlong jsplash, jintArray data, 
                                    jint x, jint y, jint width, jint height, 
                                    jint stride)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);
    int dataSize;

    if (!splash) {
        return;
    }
    SplashLock(splash);
    dataSize = (*env)->GetArrayLength(env, data);
    if (splash->overlayData) {
        free(splash->overlayData);
    }
    splash->overlayData = malloc(dataSize * sizeof(rgbquad_t));
    if (splash->overlayData) {
        /* we need a copy anyway, so we'll be using GetIntArrayRegion */
        (*env)->GetIntArrayRegion(env, data, 0, dataSize, 
            (jint *) splash->overlayData);
        initFormat(&splash->overlayFormat, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
        initRect(&splash->overlayRect, x, y, width, height, 1, 
            stride * sizeof(rgbquad_t), splash->overlayData, 
            &splash->overlayFormat);
        SplashUpdate(splash);
    }
    SplashUnlock(splash);
}


/*
* Class:     java_awt_SplashScreen
* Method:    _isVisible
* Signature: (J)Z
*/
JNIEXPORT jboolean JNICALL 
Java_java_awt_SplashScreen__1isVisible(JNIEnv * env, jclass thisClass, 
                                       jlong jsplash)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);

    if (!splash) {
        return JNI_FALSE;
    }
    return splash->isVisible>0 ? JNI_TRUE : JNI_FALSE;
}

/*
* Class:     java_awt_SplashScreen
* Method:    _getBounds
* Signature: (J)Ljava/awt/Rectangle;
*/
JNIEXPORT jobject JNICALL 
Java_java_awt_SplashScreen__1getBounds(JNIEnv * env, jclass thisClass, 
                                       jlong jsplash)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);
    static jclass clazz = NULL;
    static jmethodID mid = NULL;
    jobject bounds = NULL;

    if (!splash) {
        return NULL;
    }
    SplashLock(splash);
    if (!clazz) {
        clazz = (*env)->FindClass(env, "java/awt/Rectangle");
        if (clazz) {
            clazz = (*env)->NewGlobalRef(env, clazz);
        }
    }
    if (clazz && !mid) {
        mid = (*env)->GetMethodID(env, clazz, "<init>", "(IIII)V");
    }
    if (clazz && mid) {
        bounds = (*env)->NewObject(env, clazz, mid, splash->x, splash->y, 
            splash->width, splash->height);
        if ((*env)->ExceptionOccurred(env)) {
            bounds = NULL;
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
    }
    SplashUnlock(splash);
    return bounds;
}

/*
* Class:     java_awt_SplashScreen
* Method:    _getInstance
* Signature: ()J
*/
JNIEXPORT jlong JNICALL 
Java_java_awt_SplashScreen__1getInstance(JNIEnv * env, jclass thisClass)
{
    return ptr_to_jlong(SplashGetInstance());
}

/*
* Class:     java_awt_SplashScreen
* Method:    _close
* Signature: (J)V
*/
JNIEXPORT void JNICALL 
Java_java_awt_SplashScreen__1close(JNIEnv * env, jclass thisClass, 
                                   jlong jsplash)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);

    if (!splash) {
        return;
    }
    SplashLock(splash);
    SplashClosePlatform(splash);
    SplashUnlock(splash);
}

/*
 * Class:     java_awt_SplashScreen
 * Method:    _getImageFileName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_awt_SplashScreen__1getImageFileName
    (JNIEnv * env, jclass thisClass, jlong jsplash) 
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);


    if (!splash || !splash->fileName) {
        return NULL;
    }
    /* splash->fileName is of type char*, but in fact it contains jchars */
    return (*env)->NewString(env, (const jchar*)splash->fileName, 
                             splash->fileNameLen);
}

/*
 * Class:     java_awt_SplashScreen
 * Method:    _getImageJarName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_awt_SplashScreen__1getImageJarName
  (JNIEnv * env, jclass thisClass, jlong jsplash)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);

    if (!splash || !splash->jarName) {
        return NULL;
    }
    /* splash->jarName is of type char*, but in fact it contains jchars */
    return (*env)->NewString(env, (const jchar*)splash->jarName, 
                             splash->jarNameLen);
}

/*
 * Class:     java_awt_SplashScreen
 * Method:    _setImageData
 * Signature: (J[B)Z
 */
JNIEXPORT jboolean JNICALL Java_java_awt_SplashScreen__1setImageData
  (JNIEnv * env, jclass thisClass, jlong jsplash, jbyteArray data)
{
    Splash *splash = (Splash *) jlong_to_ptr(jsplash);
    int size, rc;
    jbyte* pBytes;

    if (!splash) {
        return JNI_FALSE;
    }
    size = (*env)->GetArrayLength(env, data);
    pBytes = (*env)->GetByteArrayElements(env, data, NULL);
    rc = SplashLoadMemory(pBytes, size);
    (*env)->ReleaseByteArrayElements(env, data, pBytes, JNI_ABORT);
    return rc ? JNI_TRUE : JNI_FALSE;
}

