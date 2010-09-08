/*
 * @(#)AccelGlyphCache.h	1.9 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AccelGlyphCache_h_Included
#define AccelGlyphCache_h_Included

#ifdef __cplusplus
extern "C" {
#endif

#include "jni.h"
#include "fontscalerdefs.h"

typedef void (FlushFunc)();

typedef struct _CacheCellInfo CacheCellInfo;

typedef struct {
    CacheCellInfo *head;
    CacheCellInfo *tail;
    unsigned int  cacheID;
    jint          width;
    jint          height;
    jint          cellWidth;
    jint          cellHeight;
    jboolean      isFull;
    FlushFunc     *Flush;
} GlyphCacheInfo;

struct _CacheCellInfo {
    GlyphCacheInfo   *cacheInfo;
    struct GlyphInfo *glyphInfo;
    // next cell info in the cache's list
    CacheCellInfo    *next;
    // REMIND: find better name?
    // next cell info in the glyph's cell list (next Glyph Cache Info)
    CacheCellInfo    *nextGCI;
    jint             timesRendered;
    jint             x;
    jint             y;
    // number of pixels from the left or right edge not considered touched
    // by the glyph
    jint             leftOff;
    jint             rightOff;
    jfloat           tx1;
    jfloat           ty1;
    jfloat           tx2;
    jfloat           ty2;
};

GlyphCacheInfo *
AccelGlyphCache_Init(jint width, jint height,
                     jint cellWidth, jint cellHeight,
                     FlushFunc *func);
CacheCellInfo *
AccelGlyphCache_AddGlyph(GlyphCacheInfo *cache, struct GlyphInfo *glyph);
void
AccelGlyphCache_Invalidate(GlyphCacheInfo *cache);
void
AccelGlyphCache_AddCellInfo(struct GlyphInfo *glyph, CacheCellInfo *cellInfo);
void
AccelGlyphCache_RemoveCellInfo(struct GlyphInfo *glyph, CacheCellInfo *cellInfo);
CacheCellInfo *
AccelGlyphCache_GetCellInfoForCache(struct GlyphInfo *glyph,
                                    GlyphCacheInfo *cache);
JNIEXPORT void
AccelGlyphCache_RemoveAllCellInfos(struct GlyphInfo *glyph);
void
AccelGlyphCache_Free(GlyphCacheInfo *cache);

#ifdef __cplusplus
};
#endif

#endif /* AccelGlyphCache_h_Included */
