/*
 * @(#)glyphblitting.h	1.5 03/23/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef BlittingIncludesDefined
#define BlittingIncludesDefined

#include "jni.h"
#include "GlyphImageRef.h"
#include "SurfaceData.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
  int numGlyphs;
  ImageRef *glyphs;
} GlyphBlitVector;

extern jint RefineBounds(GlyphBlitVector *gbv, SurfaceDataBounds *bounds);
extern GlyphBlitVector* setupBlitVector(JNIEnv *env, jobject glyphlist);
extern GlyphBlitVector* setupLCDBlitVector(JNIEnv *env, jobject glyphlist);

#ifdef	__cplusplus
}
#endif


#endif
