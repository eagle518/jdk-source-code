/*
 * @(#)cursor.c	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifdef HEADLESS
    #error This file should not be included in headless library
#endif

#include "awt_p.h"
#include "java_awt_Cursor.h"
#include "awt_Cursor.h"
#include "sun_awt_motif_MCustomCursor.h"

#include "jni.h"
#include "jni_util.h"

extern struct CursorIDs cursorIDs;
static jfieldID widthID;
static jfieldID heightID;

/*
 * Class:     sun_awt_motif_MCustomCursor
 * Method:    cacheInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCustomCursor_cacheInit
  (JNIEnv *env, jclass cls)
{
    jclass clsDimension = (*env)->FindClass(env, "java/awt/Dimension");
    widthID = (*env)->GetFieldID(env, clsDimension, "width", "I");
    heightID = (*env)->GetFieldID(env, clsDimension, "height", "I");
}

/*
 * Class:     sun_awt_motif_MCustomCursor
 * Method:    queryBestCursor
 * Signature: (Ljava/awt/Dimension;)V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCustomCursor_queryBestCursor
  (JNIEnv *env, jclass cls, jobject dimension)
{
    Window root;
    uint32_t width, height;
    
    AWT_LOCK();
    root = RootWindow(awt_display, DefaultScreen(awt_display));
    XQueryBestCursor(awt_display, root, 
                     (*env)->GetIntField(env, dimension, widthID),
                     (*env)->GetIntField(env, dimension, heightID),
                     &width, &height);
    (*env)->SetIntField(env, dimension, widthID, (int32_t) width);
    (*env)->SetIntField(env, dimension, heightID, (int32_t) height);
    AWT_UNLOCK();
}

/*
 * Class:     sun_awt_motif_MCustomCursor
 * Method:    createCursor
 * Signature: ([B[BIIII)V
 */
JNIEXPORT void JNICALL Java_sun_awt_motif_MCustomCursor_createCursor
  (JNIEnv *env , jobject this, jbyteArray xorMask, jbyteArray andMask, 
   jint width, jint height, jint fc, jint bc, jint xHotSpot, jint yHotSpot)
{
    Cursor cursor;
    char *sourceBits, *maskBits;
    Window root;
    Pixmap source, mask;
    XColor fcolor, bcolor;
    AwtGraphicsConfigDataPtr defaultConfig =
        getDefaultConfig(DefaultScreen(awt_display));

    AWT_LOCK();

    root = RootWindow(awt_display, DefaultScreen(awt_display));
    fcolor.flags = DoRed | DoGreen | DoBlue;
    fcolor.red = ((fc >> 16) & 0x000000ff) << 8;
    fcolor.green = ((fc >> 8) & 0x000000ff) << 8;
    fcolor.blue = ((fc >> 0) & 0x000000ff) << 8;
    XAllocColor(awt_display, defaultConfig->awt_cmap, &fcolor);
    bcolor.flags = DoRed | DoGreen | DoBlue;
    bcolor.red = ((bc >> 16) & 0x000000ff) << 8;
    bcolor.green = ((bc >> 8) & 0x000000ff) << 8;
    bcolor.blue = ((bc >> 0) & 0x000000ff) << 8;
    XAllocColor(awt_display, defaultConfig->awt_cmap, &bcolor);

    /* Create source pixmap. */
    sourceBits = (char *)(*env)->GetPrimitiveArrayCritical(env, xorMask, NULL);
    source = XCreateBitmapFromData(awt_display, root, sourceBits,
                                         width, height);

    /* Create mask pixmap */
    maskBits = (char *)(*env)->GetPrimitiveArrayCritical(env, andMask, NULL);
    mask = XCreateBitmapFromData(awt_display, root, maskBits,
                                       width, height);

    /* Create cursor */
    cursor = XCreatePixmapCursor(awt_display, source, mask, &fcolor, &bcolor,
                                 xHotSpot, yHotSpot);

    /* Free resources */
    XFreePixmap(awt_display, source);
    XFreePixmap(awt_display, mask);

    (*env)->ReleasePrimitiveArrayCritical(env, xorMask, sourceBits, JNI_ABORT);
    (*env)->ReleasePrimitiveArrayCritical(env, andMask, maskBits, JNI_ABORT);

	JNU_SetLongFieldFromPtr(env, this, cursorIDs.pData, cursor);

    AWT_FLUSH_UNLOCK();
}
