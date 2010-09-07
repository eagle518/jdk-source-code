/*
 * @(#)awt_Canvas.c	1.45 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "java_awt_Canvas.h"
#include "sun_awt_motif_MCanvasPeer.h"
#include "sun_awt_motif_MComponentPeer.h"
#include "color.h"
#include "canvas.h"
#include "awt_util.h"

#include "awt_Component.h"
#include "awt_GraphicsEnv.h"

#include <jni.h>
#include <jni_util.h>
#include "multi_font.h"

extern struct MComponentPeerIDs mComponentPeerIDs;
extern struct X11GraphicsConfigIDs x11GraphicsConfigIDs;
extern AwtGraphicsConfigDataPtr
    copyGraphicsConfigToPeer(JNIEnv *env, jobject this);
struct CanvasIDs mCanvasIDs;

/*
 * Class:     sun_awt_motif_MCanvasPeer
 * Method:    create
 * Signature: (Lsun/awt/motif/MComponentPeer;)V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCanvasPeer_create
  (JNIEnv * env, jobject this, jobject parent)
{
    AwtGraphicsConfigDataPtr awtData;

    struct CanvasData *wdata;
    struct CanvasData *cdata;
    jobject globalRef = awtJNI_CreateAndSetGlobalRef(env, this);

    AWT_LOCK();
    if (JNU_IsNull(env, parent)) {
        JNU_ThrowNullPointerException(env, "NullPointerException");
        AWT_UNLOCK();
        return;
    }

    cdata = (struct CanvasData *)
	JNU_GetLongFieldAsPtr(env, parent, mComponentPeerIDs.pData);
    if (cdata == NULL) {
        JNU_ThrowNullPointerException(env, "NullPointerException");
        AWT_UNLOCK();
        return;
    }

    wdata = ZALLOC(CanvasData);
    if (wdata == NULL) {
        JNU_ThrowOutOfMemoryError(env, "OutOfMemoryError");
        AWT_UNLOCK();
        return;
    }
    JNU_SetLongFieldFromPtr(env, this, mComponentPeerIDs.pData, wdata);

    awtData = copyGraphicsConfigToPeer(env, this);
    
    wdata->comp.widget = awt_canvas_create((XtPointer) globalRef,
                                           cdata->comp.widget,
					   "",
                                           1, 1, False, NULL, awtData);
    XtVaSetValues(wdata->comp.widget,
                  XmNinsertPosition, awt_util_insertCallback,
                  NULL);

    /* Add an event handler so that we can track focus change requests
       which will be initiated by Motif in response to ButtonPress events */

    wdata->flags = 0;
    wdata->shell = cdata->shell;

    AWT_UNLOCK();
}

/*
 * Class:     sun_awt_motif_MCanvasPeer
 * Method:    resetTargetGC
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCanvasPeer_resetTargetGC
(JNIEnv * env, jobject this, jobject target)
{
    (*env)->CallVoidMethod(env, target, mCanvasIDs.setGCFromPeerMID);
}

/*
 * Class:     sun_awt_motif_MCanvasPeer
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCanvasPeer_initIDs
(JNIEnv * env, jclass cls)
{
    jclass canvasCls = (*env)->FindClass(env, "java/awt/Canvas");
    mCanvasIDs.setGCFromPeerMID =
     (*env)->GetMethodID(env, canvasCls, "setGCFromPeer","()V");

    DASSERT(mCanvasIDs.setGCFromPeerMID);
}
