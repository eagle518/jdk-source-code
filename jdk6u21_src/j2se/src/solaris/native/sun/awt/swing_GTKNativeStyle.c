/*
 * @(#)swing_GTKNativeEngine.c	1.4 04/12/17
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include "gtk2_interface.h"
#include "com_sun_java_swing_plaf_gtk_GTKNativeStyle.h"

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_xthickness
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1xthickness(
        JNIEnv *env, jobject this, jint widget_type)
{
    return gtk2_get_xthickness(env, widget_type);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_ythickness
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1ythickness(
        JNIEnv *env, jobject this, jint widget_type)
{
    return gtk2_get_ythickness(env, widget_type);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_color_for_state
 * Signature: (III)I
 */
JNIEXPORT jint JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1color_1for_1state(
        JNIEnv *env, jobject this, jint widget_type, jint state_type, jint type_id)
{
    return gtk2_get_color_for_state(env, widget_type, state_type, type_id);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_class_value
 * Signature: (ILjava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1class_1value(
        JNIEnv *env, jobject this, jint widget_type, jstring key)
{
    return gtk2_get_class_value(env, widget_type, key);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_pango_font_name
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1pango_1font_1name(
        JNIEnv *env, jobject this, jint widget_type)
{
    return gtk2_get_pango_font_name(env, widget_type);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_image_dimension
 * Signature: (II)Ljava/awt/Dimension;
 */
JNIEXPORT jobject JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1image_1dimension(
        JNIEnv *env, jobject this, jint widget_type, jint state_type)
{
    return gtk2_get_image_dimension(env, widget_type, state_type);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeStyle
 * Method:    native_get_image
 * Signature: ([IIIII)V
 */
JNIEXPORT void JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeStyle_native_1get_1image(
        JNIEnv *env, jobject this, jintArray dest, jint width, jint height,
        jint widget_type, jint state_type)
{
    gint *buffer = (gint*) (*env)->GetPrimitiveArrayCritical(env, dest, 0);
    gtk2_get_image(buffer, width, height, widget_type, state_type);
    (*env)->ReleasePrimitiveArrayCritical(env, dest, buffer, 0);
}
