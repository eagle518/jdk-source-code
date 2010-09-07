/*
 * @(#)AccelGlyphCache.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AccelGlyphCache_h_Included
#define AccelGlyphCache_h_Included

#include "jni.h"
#include "fontscalerdefs.h"

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
} GlyphCacheInfo;

struct _CacheCellInfo {
    GlyphCacheInfo   *cacheInfo;
    struct GlyphInfo *glyphInfo;
    CacheCellInfo    *next;
    jint             timesRendered;
    jint             x;
    jint             y;
    jfloat           tx1;
    jfloat           ty1;
    jfloat           tx2;
    jfloat           ty2;
};

GlyphCacheInfo *
AccelGlyphCache_Init(jint width, jint height,
                     jint cellWidth, jint cellHeight);
void
AccelGlyphCache_AddGlyph(GlyphCacheInfo *cache, struct GlyphInfo *glyph);

#endif /* AccelGlyphCache_h_Included */
