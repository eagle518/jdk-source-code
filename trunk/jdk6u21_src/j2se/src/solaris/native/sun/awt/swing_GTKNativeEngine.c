/*
 * @(#)swing_GTKNativeEngine.c	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include "gtk2_interface.h"
#include "com_sun_java_swing_plaf_gtk_GTKNativeEngine.h"

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_arrow
 * Signature: (IIILjava/lang/String;IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1arrow(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, jint arrow_type)
{
    gtk2_paint_arrow(widget_type, state, shadow_type, getStrFor(env, detail),
            x, y, w, h, arrow_type, TRUE);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_box
 * Signature: (IIILjava/lang/String;IIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1box(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, 
        jint synth_state, jint dir)
{
    gtk2_paint_box(widget_type, state, shadow_type, getStrFor(env, detail),
                   x, y, w, h, synth_state, dir);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_box_gap
 * Signature: (IIILjava/lang/String;IIIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1box_1gap(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h,
        jint gap_side, jint gap_x, jint gap_w)
{
    gtk2_paint_box_gap(widget_type, state, shadow_type, getStrFor(env, detail),
            x, y, w, h, gap_side, gap_x, gap_w);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_check
 * Signature: (IILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1check(
        JNIEnv *env, jobject this,
        jint widget_type, jint synth_state, jstring detail,
        jint x, jint y, jint w, jint h)
{
    gtk2_paint_check(widget_type, synth_state, getStrFor(env, detail),
                     x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_expander
 * Signature: (IILjava/lang/String;IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1expander(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jstring detail,
        jint x, jint y, jint w, jint h, jint expander_style)
{
    gtk2_paint_expander(widget_type, state, getStrFor(env, detail),
            x, y, w, h, expander_style);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_extension
 * Signature: (IIILjava/lang/String;IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1extension(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, jint placement)
{
    gtk2_paint_extension(widget_type, state, shadow_type,
            getStrFor(env, detail), x, y, w, h, placement);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_flat_box
 * Signature: (IIILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1flat_1box(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, jboolean has_focus)
{
    gtk2_paint_flat_box(widget_type, state, shadow_type,
            getStrFor(env, detail), x, y, w, h, has_focus);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_focus
 * Signature: (IILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1focus(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jstring detail,
        jint x, jint y, jint w, jint h)
{
    gtk2_paint_focus(widget_type, state, getStrFor(env, detail),
            x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_handle
 * Signature: (IIILjava/lang/String;IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1handle(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, jint orientation)
{
    gtk2_paint_handle(widget_type, state, shadow_type, getStrFor(env, detail),
            x, y, w, h, orientation);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_hline
 * Signature: (IILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1hline(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jstring detail,
        jint x, jint y, jint w, jint h)
{
    gtk2_paint_hline(widget_type, state, getStrFor(env, detail),
            x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_option
 * Signature: (IILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1option(
        JNIEnv *env, jobject this,
        jint widget_type, jint synth_state, jstring detail,
        jint x, jint y, jint w, jint h)
{
    gtk2_paint_option(widget_type, synth_state, getStrFor(env, detail),
                      x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_shadow
 * Signature: (IIILjava/lang/String;IIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1shadow(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h,
        jint synth_state, jint dir)
{
    gtk2_paint_shadow(widget_type, state, shadow_type, getStrFor(env, detail),
                      x, y, w, h, synth_state, dir);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_slider
 * Signature: (IIILjava/lang/String;IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1slider(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jint shadow_type, jstring detail,
        jint x, jint y, jint w, jint h, jint orientation)
{
    gtk2_paint_slider(widget_type, state, shadow_type, getStrFor(env, detail),
            x, y, w, h, orientation);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_vline
 * Signature: (IILjava/lang/String;IIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1vline(
        JNIEnv *env, jobject this,
        jint widget_type, jint state, jstring detail,
        jint x, jint y, jint w, jint h)
{
    gtk2_paint_vline(widget_type, state, getStrFor(env, detail),
            x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_paint_background
 * Signature: (IIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1paint_1background(
        JNIEnv *env, jobject this, jint widget_type, jint state,
        jint x, jint y, jint w, jint h)
{
    gtk_paint_background(widget_type, state, x, y, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    nativeStartPainting
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_nativeStartPainting(
        JNIEnv *env, jobject this, jint w, jint h)
{
    gtk2_init_painting(env, w, h);
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    nativeFinishPainting
 * Signature: ([III)I
 */
JNIEXPORT jint JNICALL
Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_nativeFinishPainting(
        JNIEnv *env, jobject this, jintArray dest, jint width, jint height)
{
    jint transparency;
    gint *buffer = (gint*) (*env)->GetPrimitiveArrayCritical(env, dest, 0);
    transparency = gtk2_copy_image(buffer, width, height);
    (*env)->ReleasePrimitiveArrayCritical(env, dest, buffer, 0);
    return transparency;
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_switch_theme
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1switch_1theme(
        JNIEnv *env, jobject this)
{
    flush_gtk_event_loop();
}

/*
 * Class:     com_sun_java_swing_plaf_gtk_GTKNativeEngine
 * Method:    native_get_gtk_setting
 * Signature: (I)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_sun_java_swing_plaf_gtk_GTKNativeEngine_native_1get_1gtk_1setting(
        JNIEnv *env, jobject this, jint property)
{
    return gtk2_get_setting(env, property);
}
