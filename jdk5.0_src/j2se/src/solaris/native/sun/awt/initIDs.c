/*
 * @(#)initIDs.c	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "java_awt_Color.h"
#include "java_awt_Dimension.h"
#include "java_awt_MenuBar.h"
#include "java_awt_Label.h"
#include "java_awt_FontMetrics.h"
#include "java_awt_event_MouseEvent.h"
#include "java_awt_Rectangle.h"
#include "java_awt_ScrollPaneAdjustable.h"
#include "java_awt_Toolkit.h"
#include "java_awt_CheckboxMenuItem.h"
#include "sun_awt_CharsetString.h"

#include "jni_util.h"

/*
 * This file contains stubs for JNI field and method id initializers
 * which are used in the win32 awt.
 */

jfieldID colorValueID;

JNIEXPORT void JNICALL
Java_java_awt_Color_initIDs
  (JNIEnv *env, jclass clazz)
{        
    colorValueID = (*env)->GetFieldID(env, clazz, "value", "I");

    if(colorValueID == NULL)
        JNU_ThrowNullPointerException (env, "Can't get java/awt/Color.value fieldID");
}

JNIEXPORT void JNICALL
Java_java_awt_MenuBar_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_Label_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_FontMetrics_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_Toolkit_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_ScrollPaneAdjustable_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_CheckboxMenuItem_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_Dimension_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_Rectangle_initIDs
  (JNIEnv *env, jclass clazz)
{
}

JNIEXPORT void JNICALL
Java_java_awt_event_MouseEvent_initIDs
  (JNIEnv *env, jclass clazz)
{
}
