/*
 * @(#)OGLTextRenderer.c	1.8 04/01/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <malloc.h>

#include "sun_java2d_opengl_OGLTextRenderer.h"

#include "SurfaceData.h"
#include "OGLSurfaceData.h"
#include "Region.h"
#include "glyphblitting.h"

JNIEXPORT void JNICALL
OGLDrawGlyphList(JNIEnv *env, jobject otr,
                 jlong pCtx,
                 ImageRef *glyphs, jint totalGlyphs, jboolean useCache);

/**
 * This method is almost exactly the same as the RefineBounds() method
 * defined in DrawGlyphList.c.  The goal is to determine whether the given
 * GlyphBlitVector intersects with the given bounding box.  If any part of
 * the GBV intersects with the bounding box, this method returns true;
 * otherwise false is returned.  The only step that differs in this method
 * from RefineBounds() is that we check to see whether all the glyphs in
 * the GBV will fit in the glyph cache.  If any glyph is too big for a
 * glyph cache cell, we return FALSE in the useCache out parameter; otherwise
 * useCache is TRUE, indicating that the caller can be assured that all
 * the glyphs can be stored in the accelerated glyph cache.
 */
static jboolean
OGLRefineBounds(GlyphBlitVector *gbv, SurfaceDataBounds *bounds,
                jboolean *useCache)
{
    int index;
    jint dx1, dy1, dx2, dy2;
    ImageRef glyphImage;
    int num = gbv->numGlyphs;
    SurfaceDataBounds glyphs;
    jboolean tryCache = JNI_TRUE;

    glyphs.x1 = glyphs.y1 = 0x7fffffff;
    glyphs.x2 = glyphs.y2 = 0x80000000;
    for (index = 0; index < num; index++) {
        glyphImage = gbv->glyphs[index];
        dx1 = (jint) glyphImage.x;
        dy1 = (jint) glyphImage.y;
        dx2 = dx1 + glyphImage.width;
        dy2 = dy1 + glyphImage.height;
	if (glyphs.x1 > dx1) glyphs.x1 = dx1;
	if (glyphs.y1 > dy1) glyphs.y1 = dy1;
	if (glyphs.x2 < dx2) glyphs.x2 = dx2;
	if (glyphs.y2 < dy2) glyphs.y2 = dy2;

        if (tryCache &&
            ((glyphImage.width > OGLSD_CACHE_CELL_WIDTH) ||
             (glyphImage.height > OGLSD_CACHE_CELL_HEIGHT)))
        {
            tryCache = JNI_FALSE;
        }
    }

    *useCache = tryCache;

    SurfaceData_IntersectBounds(bounds, &glyphs);
    return (bounds->x1 < bounds->x2 && bounds->y1 < bounds->y2);
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_OGLTextRenderer_doDrawGlyphList
    (JNIEnv *env, jobject otr,
     jlong pCtx,
     jobject clip, jobject glyphlist)
{
    GlyphBlitVector* gbv;
    SurfaceDataBounds bounds;
    jboolean useCache;

    SurfaceData_GetBoundsFromRegion(env, clip, &bounds);

    if ((gbv = setupBlitVector(env, glyphlist)) == NULL) {
	return;
    }
    if (!OGLRefineBounds(gbv, &bounds, &useCache)) {
        free(gbv);
        return;
    }

    OGLDrawGlyphList(env, otr,
                     pCtx, gbv->glyphs, gbv->numGlyphs, useCache);
    free(gbv);
}
