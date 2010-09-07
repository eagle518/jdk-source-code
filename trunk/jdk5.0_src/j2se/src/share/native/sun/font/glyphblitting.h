/*
 * @(#)glyphblitting.h	1.2 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

#ifdef	__cplusplus
}
#endif


#endif
