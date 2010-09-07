/*
 * @(#)awt_GlobalCursorManager.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "awt_Component.h"
#include "sun_awt_motif_MComponentPeer.h"

#include "jni.h"
#include "jni_util.h"

static jfieldID xID;
static jfieldID yID;
 
extern struct MComponentPeerIDs mComponentPeerIDs;
extern struct ComponentIDs componentIDs;
extern struct ContainerIDs containerIDs;
extern jobject getCurComponent();

/*
 * Class:     sun_awt_motif_MGlobalCursorManager
 * Method:    cacheInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MGlobalCursorManager_cacheInit
  (JNIEnv *env, jclass cls)
{
    jclass clsDimension = (*env)->FindClass(env, "java/awt/Point");
    xID = (*env)->GetFieldID(env, clsDimension, "x", "I");
    yID = (*env)->GetFieldID(env, clsDimension, "y", "I");
}

/*
 * Class:     sun_awt_motif_MGlobalCursorManager
 * Method:    getCursorPos
 * Signature: (Ljava/awt/Point;)Ljava/awt/Component
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MGlobalCursorManager_getCursorPos
  (JNIEnv *env, jobject this, jobject point)
{
    Window root, rw, cw;
    int32_t rx, ry, x, y;
    uint32_t kbs;
    
    AWT_LOCK();
    root = RootWindow(awt_display, DefaultScreen(awt_display));
    XQueryPointer(awt_display, root, &rw, &cw, &rx, &ry, &x, &y, &kbs);

    (*env)->SetIntField(env, point, xID, rx);
    (*env)->SetIntField(env, point, yID, ry);
    AWT_FLUSH_UNLOCK();
}

/*
 * Class:     sun_awt_motif_MGlobalCursorManager
 * Method:    getCursorPos
 * Signature: ()Ljava/awt/Component
 */
JNIEXPORT jobject JNICALL Java_sun_awt_motif_MGlobalCursorManager_findHeavyweightUnderCursor
  (JNIEnv *env, jobject this)
{
	jobject target;

    AWT_LOCK();
	target = getCurComponent();
    AWT_FLUSH_UNLOCK();
	return target;		
}

/*
 * Class:     sun_awt_motif_MGlobalCursorManager
 * Method:    getLocationOnScreen
 * Signature: (Ljava/awt/Component;)Ljava/awt/Point
 */
JNIEXPORT jobject JNICALL Java_sun_awt_motif_MGlobalCursorManager_getLocationOnScreen
  (JNIEnv *env, jobject this, jobject component)
{
    jobject point =
        (*env)->CallObjectMethod(env, component,
				 componentIDs.getLocationOnScreen);
    return point;
}

/*
 * Class:     sun_awt_motif_MGlobalCursorManager
 * Method:    findComponentAt
 * Signature: (Ljava/awt/Container;II)Ljava/awt/Component
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_motif_MGlobalCursorManager_findComponentAt
    (JNIEnv *env, jobject this, jobject container, jint x, jint y)
{
    /*
     * Call private version of Container.findComponentAt with the following
     * flag set: ignoreEnabled = false (i.e., don't return or recurse into
     * disabled Components).  
     * NOTE: it may return a JRootPane's glass pane as the target Component.
     */
    jobject component =
        (*env)->CallObjectMethod(env, container, containerIDs.findComponentAt,
				 x, y, JNI_FALSE);
    return component;
}

