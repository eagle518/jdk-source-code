/*
 * @(#)OGLTextRenderer_md.c	1.10 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <malloc.h>
#include <jlong.h>

#include "SurfaceData.h"
#include "OGLSurfaceData.h"
#include "AccelGlyphCache.h"
#include "glyphblitting.h"
#include "fontscalerdefs.h"
#include "j2d_md.h"

#ifndef HEADLESS

static GlyphCacheInfo *glyphCache = NULL;

static GlyphCacheInfo *
OGLGlyphCache_Init(JNIEnv *env, OGLContext *oglc,
                   jint width, jint height, jint cellWidth, jint cellHeight)
{
    GlyphCacheInfo *gcinfo;
    GLclampf priority = 1.0f;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLGlyphCache_Init");

    // init glyph cache data structure
    gcinfo = AccelGlyphCache_Init(width, height, cellWidth, cellHeight);
    if (gcinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not init OGL glyph cache");
        return NULL;
    }

    // init cache texture object
    j2d_glGenTextures(1, &gcinfo->cacheID);
    j2d_glBindTexture(GL_TEXTURE_2D, gcinfo->cacheID);
    j2d_glPrioritizeTextures(1, &gcinfo->cacheID, &priority);

    j2d_glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY,
                     width, height, 0,
                     GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    return gcinfo;
}

static void
OGLGlyphCache_Add(JNIEnv *env, GlyphInfo *glyph)
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLGlyphCache_Add");

    if ((glyphCache == NULL) || (glyph->image == NULL)) {
        return;
    }

    AccelGlyphCache_AddGlyph(glyphCache, glyph);

    if (glyph->cellInfo != NULL) {
        // store glyph image in texture cell
        j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                            glyph->cellInfo->x, glyph->cellInfo->y,
                            glyph->width, glyph->height,
                            GL_LUMINANCE, GL_UNSIGNED_BYTE, glyph->image);
    }
}

/**
 * Renders each glyph directly from the glyph texture cache.
 */
static void
OGLDrawGlyphList_UseCache(JNIEnv *env, OGLContext *oglc,
                          ImageRef *glyphs, jint totalGlyphs)
{
    int glyphCounter;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLDrawGlyphList_UseCache");

    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glBindTexture(GL_TEXTURE_2D, glyphCache->cacheID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    j2d_glBegin(GL_QUADS);

    for (glyphCounter = 0; glyphCounter < totalGlyphs; glyphCounter++) {
        // render glyph cached in texture object
        const jubyte *pixels = (const jubyte *)glyphs[glyphCounter].pixels;
        GlyphInfo *ginfo = (GlyphInfo *)glyphs[glyphCounter].glyphInfo;
        CacheCellInfo *cell;
        GLfloat x1, y1, x2, y2;

        if (!pixels) {
            continue;
        }

        if (ginfo->cellInfo == NULL) {
            j2d_glEnd();

            // attempt to add glyph to accelerated glyph cache
            OGLGlyphCache_Add(env, ginfo);

            j2d_glBegin(GL_QUADS);

            if (ginfo->cellInfo == NULL) {
                // we'll just no-op in the rare case that the cell is NULL
                continue;
            }
        }

        cell = ginfo->cellInfo;
        cell->timesRendered++;

        x1 = (GLfloat)glyphs[glyphCounter].x;
        y1 = (GLfloat)glyphs[glyphCounter].y;
        x2 = x1 + (GLfloat)glyphs[glyphCounter].width;
        y2 = y1 + (GLfloat)glyphs[glyphCounter].height;

        j2d_glTexCoord2f(cell->tx1, cell->ty1); j2d_glVertex2f(x1, y1);
        j2d_glTexCoord2f(cell->tx2, cell->ty1); j2d_glVertex2f(x2, y1);
        j2d_glTexCoord2f(cell->tx2, cell->ty2); j2d_glVertex2f(x2, y2);
        j2d_glTexCoord2f(cell->tx1, cell->ty2); j2d_glVertex2f(x1, y2);
    }

    j2d_glEnd();
    j2d_glDisable(GL_TEXTURE_2D);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    OGLContext_Flush(env, oglc);
}

/**
 * Renders each glyph by first uploading the system memory glyph image into
 * a texture tile and then from that texture to the destination surface
 * (this technique is very similar to the one employed in OGLMaskFill).
 */
static void
OGLDrawGlyphList_NoCache(JNIEnv *env, OGLContext *oglc,
                         ImageRef *glyphs, jint totalGlyphs)
{
    int glyphCounter;
    GLfloat tx1, ty1, tx2, ty2;
    jint tw, th;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLDrawGlyphList_NoCache");

    if (oglc->maskTextureID == 0) {
        if (OGLSD_InitMaskTileTexture(oglc) == SD_FAILURE) {
            return;
        }
    }

    j2d_glEnable(GL_TEXTURE_2D);
    j2d_glBindTexture(GL_TEXTURE_2D, oglc->maskTextureID);
    j2d_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    j2d_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    tx1 = 0.0f;
    ty1 = 0.0f;
    tw = OGLSD_MASK_TILE_SIZE;
    th = OGLSD_MASK_TILE_SIZE;

    for (glyphCounter = 0; glyphCounter < totalGlyphs; glyphCounter++) {
        // render system memory glyph image
        jint sx, sy, sw, sh;
        jint x, y, w, h, x0;
        const jubyte *pixels = (const jubyte *)glyphs[glyphCounter].pixels;

        if (!pixels) {
            continue;
        }

        x = glyphs[glyphCounter].x;
        y = glyphs[glyphCounter].y;
        w = glyphs[glyphCounter].width;
        h = glyphs[glyphCounter].height;
        x0 = x;

        j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, w);

        for (sy = 0; sy < h; sy += th, y += th) {
            x = x0;
            sh = ((sy + th) > h) ? (h - sy) : th;

            for (sx = 0; sx < w; sx += tw, x += tw) {
                sw = ((sx + tw) > w) ? (w - sx) : tw;

                // update the source pointer offsets
                j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, sx);
                j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, sy);

                // copy alpha mask into texture tile
                j2d_glTexSubImage2D(GL_TEXTURE_2D, 0,
                                    0, 0, sw, sh,
                                    GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);

                // update the lower right texture coordinates
                tx2 = ((GLfloat)sw) / tw;
                ty2 = ((GLfloat)sh) / th;

                // render texture tile to the destination surface
                j2d_glBegin(GL_QUADS);
                j2d_glTexCoord2f(tx1, ty1); j2d_glVertex2i(x, y);
                j2d_glTexCoord2f(tx2, ty1); j2d_glVertex2i(x + sw, y);
                j2d_glTexCoord2f(tx2, ty2); j2d_glVertex2i(x + sw, y + sh);
                j2d_glTexCoord2f(tx1, ty2); j2d_glVertex2i(x, y + sh);
                j2d_glEnd();
            }
        }
    }

    j2d_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    j2d_glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    j2d_glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    j2d_glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    j2d_glDisable(GL_TEXTURE_2D);

    OGLContext_Flush(env, oglc);
}

#endif /* !HEADLESS */

JNIEXPORT void JNICALL
OGLDrawGlyphList(JNIEnv *env, jobject otr,
                 jlong pCtx,
                 ImageRef *glyphs, jint totalGlyphs,
                 jboolean useCache)
{
#ifndef HEADLESS
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    static jboolean cacheAvailable = JNI_TRUE;

    J2dTraceLn(J2D_TRACE_INFO, "in OGLDrawGlyphList");

    if (oglc == NULL) {
        return;
    }

    if (useCache && cacheAvailable) {
        if (glyphCache == NULL) {
            // initialize the glyph cache
            glyphCache = OGLGlyphCache_Init(env, oglc,
                                            OGLSD_CACHE_WIDTH,
                                            OGLSD_CACHE_HEIGHT,
                                            OGLSD_CACHE_CELL_WIDTH,
                                            OGLSD_CACHE_CELL_HEIGHT);
            if (glyphCache == NULL) {
                // disable future caching attempts and use the no-cache method
                cacheAvailable = JNI_FALSE;
            }
        }

        if (glyphCache != NULL) {
            // render glyphs using the accelerated cache
            OGLDrawGlyphList_UseCache(env, oglc,
                                      glyphs, totalGlyphs);
            return;
        }
    }

    // render glyphs using the system memory glyph image and a texture tile
    OGLDrawGlyphList_NoCache(env, oglc,
                             glyphs, totalGlyphs);
#endif /* !HEADLESS */
}
