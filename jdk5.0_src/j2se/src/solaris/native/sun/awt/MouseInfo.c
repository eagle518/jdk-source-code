/*
 * @(#)MouseInfo.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "awt_Component.h"

#include <jni.h>
#include <jni_util.h>

extern int awt_numScreens;
extern AwtScreenDataPtr x11Screens;
extern struct ComponentIDs componentIDs;
extern struct MComponentPeerIDs mComponentPeerIDs;

/*
 * Class:     sun_awt_DefaultMouseInfoPeer
 * Method:    fillPointWithCoords
 * Signature: (Ljava/awt/Point)I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_DefaultMouseInfoPeer_fillPointWithCoords(JNIEnv *env, jclass cls,
                                                          jobject point)
{
     static jclass pointClass = NULL;
     jclass pointClassLocal;
     static jfieldID xID, yID;
     Window rootWindow, childWindow;
     int i;
     int32_t xr, yr, xw, yw;
     uint32_t keys;
     BOOL pointerFound;

     AWT_LOCK();
     if (pointClass == NULL) {
         pointClassLocal = (*env)->FindClass(env, "java/awt/Point");
         DASSERT(pointClassLocal != NULL);
         if (pointClassLocal == NULL) {
             AWT_UNLOCK();
             return (jint)0;
         }
         pointClass = (jclass)(*env)->NewGlobalRef(env, pointClassLocal);
         (*env)->DeleteLocalRef(env, pointClassLocal);
         xID = (*env)->GetFieldID(env, pointClass, "x", "I");
         yID = (*env)->GetFieldID(env, pointClass, "y", "I");
     }

     for (i = 0; i < awt_numScreens; i++) {
         pointerFound = XQueryPointer(awt_display, x11Screens[i].root,
                           &rootWindow, &childWindow,
                           &xr, &yr, &xw, &yw, &keys);
         if (pointerFound) {
             (*env)->SetIntField(env, point, xID, xr);
             (*env)->SetIntField(env, point, yID, yr);
             AWT_UNLOCK();
             return (jint)i;
         }
     }
     /* This should never happen */
     DASSERT(FALSE);
     AWT_UNLOCK();
     return (jint)0;
}

/*
 * Class:     sun_awt_DefaultMouseInfoPeer
 * Method:    isWindowUnderMouse
 * Signature: (Ljava/awt/Window)Z
 */
JNIEXPORT jboolean JNICALL Java_sun_awt_DefaultMouseInfoPeer_isWindowUnderMouse
  (JNIEnv * env, jclass cls, jobject window)
{
    Window rootWindow, childWindow;
    int i;
    int32_t xr, yr, xw, yw;
    uint32_t keys;
    BOOL pointerFound;
    struct FrameData *wdata;
    jobject winPeer;

    if ((*env)->EnsureLocalCapacity(env, 1) < 0) {
        return JNI_FALSE;
    }
    winPeer = (*env)->GetObjectField(env, window, componentIDs.peer);
    if (JNU_IsNull(env, winPeer)) {
        return JNI_FALSE;
    }
    wdata = (struct FrameData *)
        JNU_GetLongFieldAsPtr(env, winPeer, mComponentPeerIDs.pData);
    (*env)->DeleteLocalRef(env, winPeer);
    if (wdata == NULL) {
        return JNI_FALSE;
    }

    AWT_LOCK();
    pointerFound = XQueryPointer(awt_display, XtWindow(wdata->winData.shell),
                                 &rootWindow, &childWindow,
                                 &xr, &yr, &xw, &yw, &keys);
    AWT_UNLOCK();
    return pointerFound ? JNI_TRUE : JNI_FALSE;
}
