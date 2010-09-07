/* Header for class sun_font_SunLayoutEngine */

#include <jni_util.h>
#include <stdlib.h>

#include "FontInstanceAdapter.h"
#include "LayoutEngine.h"
#include "SunLayoutEngine.h"
#include "sunfontids.h"

void getFloat(JNIEnv* env, jobject pt, jfloat &x, jfloat &y) {
    x = env->GetFloatField(pt, sunFontIDs.xFID);
    y = env->GetFloatField(pt, sunFontIDs.yFID);
}

void putFloat(JNIEnv* env, jobject pt, jfloat x, jfloat y) {
    env->SetFloatField(pt, sunFontIDs.xFID, x);
    env->SetFloatField(pt, sunFontIDs.yFID, y);
}

static jclass gvdClass = 0;
static const char* gvdClassName = "sun/font/GlyphLayout$GVData";
static jfieldID gvdCountFID = 0;
static jfieldID gvdFlagsFID = 0;
static jfieldID gvdGlyphsFID = 0;
static jfieldID gvdPositionsFID = 0;
static jfieldID gvdIndicesFID = 0;

JNIEXPORT void JNICALL
Java_sun_font_SunLayoutEngine_initGVIDs
    (JNIEnv *env, jclass cls) {
    gvdClass = env->FindClass(gvdClassName);
    if (!gvdClass) {
	JNU_ThrowClassNotFoundException(env, gvdClassName);
	return;
    }
    gvdClass = (jclass)env->NewGlobalRef(gvdClass);
      if (!gvdClass) {
	JNU_ThrowInternalError(env, "could not create global ref");
	return;
    }
    gvdCountFID = env->GetFieldID(gvdClass, "_count", "I");
    if (!gvdCountFID) {
      gvdClass = 0;
      JNU_ThrowNoSuchFieldException(env, "_count");
      return;
    }

    gvdFlagsFID = env->GetFieldID(gvdClass, "_flags", "I");
    if (!gvdFlagsFID) {
      gvdClass = 0;
      JNU_ThrowNoSuchFieldException(env, "_flags");
      return;
    }

    gvdGlyphsFID = env->GetFieldID(gvdClass, "_glyphs", "[I");
    if (!gvdGlyphsFID) {
      gvdClass = 0;
      JNU_ThrowNoSuchFieldException(env, "_glyphs");
      return;
    }

    gvdPositionsFID = env->GetFieldID(gvdClass, "_positions", "[F");
    if (!gvdPositionsFID) {
      gvdClass = 0;
      JNU_ThrowNoSuchFieldException(env, "_positions");
      return;
    }

    gvdIndicesFID = env->GetFieldID(gvdClass, "_indices", "[I");
    if (!gvdIndicesFID) {
      gvdClass = 0;
      JNU_ThrowNoSuchFieldException(env, "_indices");
      return;
    }
}

int putGV(JNIEnv* env, jint gmask, jint baseIndex, jobject gvdata, const LayoutEngine* engine, int glyphCount) {
    int count = env->GetIntField(gvdata, gvdCountFID);

    jarray glyphArray = (jarray)env->GetObjectField(gvdata, gvdGlyphsFID);
    if (IS_NULL(glyphArray)) {
      JNU_ThrowInternalError(env, "glypharray null");
      return 0;
    }
    jint capacity = env->GetArrayLength(glyphArray);
    if (count + glyphCount > capacity) {
      JNU_ThrowArrayIndexOutOfBoundsException(env, "");
      return 0;
    }

    jarray posArray = (jarray)env->GetObjectField(gvdata, gvdPositionsFID);
    if (IS_NULL(glyphArray)) {
      JNU_ThrowInternalError(env, "positions array null");
      return 0;
    }
    jarray inxArray = (jarray)env->GetObjectField(gvdata, gvdIndicesFID);
    if (IS_NULL(inxArray)) {
      JNU_ThrowInternalError(env, "indices array null");
      return 0;
    }
      
    // le_uint32 is the same size as jint... forever, we hope
    le_uint32* glyphs = (le_uint32*)env->GetPrimitiveArrayCritical(glyphArray, NULL);
    if (glyphs) {
      jfloat* positions = (jfloat*)env->GetPrimitiveArrayCritical(posArray, NULL);
      if (positions) {
        jint* indices = (jint*)env->GetPrimitiveArrayCritical(inxArray, NULL);
        if (indices) {
          LEErrorCode status = (LEErrorCode)0;
          engine->getGlyphs(glyphs + count, gmask, status);
          engine->getGlyphPositions(positions + (count * 2), status);
          engine->getCharIndices((le_int32*)(indices + count), baseIndex, status);

          count += glyphCount;
          env->SetIntField(gvdata, gvdCountFID, count);

          // !!! need engine->getFlags to signal positions, indices data
	  /* "0" arg used instead of JNI_COMMIT as we want the carray
	   * to be freed by any VM that actually passes us a copy.
	   */
          env->ReleasePrimitiveArrayCritical(inxArray, indices, 0);
        }
        env->ReleasePrimitiveArrayCritical(posArray, positions, 0);
      }
      env->ReleasePrimitiveArrayCritical(glyphArray, glyphs, 0);
    }

  return 1;
}

/*
 * Class:     sun_font_SunLayoutEngine
 * Method:    nativeLayout
 * Signature: (Lsun/font/FontStrike;[CIIIIZLjava/awt/geom/Point2D$Float;Lsun/font/GlyphLayout$GVData;)V
 */
JNIEXPORT void JNICALL Java_sun_font_SunLayoutEngine_nativeLayout
   (JNIEnv *env, jclass cls, jobject font2d, jobject strike, jfloatArray matrix, jint gmask,
   jint baseIndex, jcharArray text, jint start, jint limit, jint min, jint max, 
   jint script, jint lang, jboolean rtl, jobject pt, jobject gvdata) 
{
    //  fprintf(stderr, "nl font: %x strike: %x script: %d\n", font2d, strike, script); fflush(stderr);
  float mat[4];
  env->GetFloatArrayRegion(matrix, 0, 4, mat);
  FontInstanceAdapter fia(env, font2d, strike, mat, 72, 72);
  LEErrorCode success = LE_NO_ERROR;
  LayoutEngine *engine = LayoutEngine::layoutEngineFactory(&fia, script, lang, success);

  // have to copy, yuck, since code does upcalls now.  this will be soooo slow
  jint len = max - min;
  jchar buffer[256];
  jchar* chars = buffer;
  if (len > 256) {
    chars = (jchar*)malloc(len * sizeof(jchar));
    if (chars == 0) {
      return;
    }
  }
  //  fprintf(stderr, "nl chars: %x text: %x min %d len %d rtl %d\n", chars, text, min, len, rtl); fflush(stderr);

  env->GetCharArrayRegion(text, min, len, chars);

  jfloat x, y;
  getFloat(env, pt, x, y);
  int glyphCount = engine->layoutChars(chars, start - min, limit - start, len, rtl, x, y, success);
  //   fprintf(stderr, "sle nl len %d -> gc: %d\n", len, glyphCount); fflush(stderr);

  engine->getGlyphPosition(glyphCount, x, y, success);

  //  fprintf(stderr, "layout glyphs: %d x: %g y: %g\n", glyphCount, x, y); fflush(stderr);

  if (putGV(env, gmask, baseIndex, gvdata, engine, glyphCount)) {
    // !!! hmmm, could use current value in positions array of GVData...
    putFloat(env, pt, x, y);
  }

  if (chars != buffer) {
    free(chars);
  }

  delete engine;

}
