/* Header for class sun_awt_font_SunLayoutEngine */

#ifndef _Included_sun_awt_font_SunLayoutEngine
#define _Included_sun_awt_font_SunLayoutEngine

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     sun_font_SunLayoutEngine
 * Method:    initGVIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_font_SunLayoutEngine_initGVIDs
  (JNIEnv *, jclass);

/*
 * Class:     sun_font_SunLayoutEngine
 * Method:    nativeLayout
 * Signature: (Lsun/font/FontStrike;[CIIIIZLjava/awt/geom/Point2D$Float;Lsun/awt/font/GlyphLayout$GVData;)V
 */
JNIEXPORT void JNICALL Java_sun_font_SunLayoutEngine_nativeLayout
   (JNIEnv *env, jclass cls, jobject font2d, jobject strike, jfloatArray matrix, jint gmask, jint baseIndex,
   jcharArray text, jint offset, jint limit, jint min, jint max, 
   jint script, jint lang, jboolean rtl, jobject pt, jobject gvdata);

#ifdef __cplusplus
}
#endif

#endif

