/*
 * @(#)AccelGlyphCache.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <malloc.h>
#include "jni.h"
#include "AccelGlyphCache.h"
#include "Trace.h"

/**
 * When the cache is full, we will try to reuse the cache cells that have
 * been used relatively less than the others (and we will save the cells that
 * have been rendered more than the threshold defined here).
 */
#define TIMES_RENDERED_THRESHOLD 5

/**
 * Creates a new GlyphCacheInfo structure, fills in the initial values, and
 * then returns a pointer to the GlyphCacheInfo record.
 *
 * Note that this method only sets up a data structure describing a
 * rectangular region of accelerated memory, containing "virtual" cells of
 * the requested size.  The cell information is added lazily to the linked
 * list describing the cache as new glyphs are added.  Platform specific
 * glyph caching code is responsible for actually creating the accelerated
 * memory surface that will contain the individual glyph images.
 */
GlyphCacheInfo *
AccelGlyphCache_Init(jint width, jint height, jint cellWidth, jint cellHeight)
{
    GlyphCacheInfo *gcinfo;

    J2dTraceLn(J2D_TRACE_INFO, "in AccelGlyphCache_Init");

    gcinfo = (GlyphCacheInfo *)malloc(sizeof(GlyphCacheInfo));
    if (gcinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not allocate GlyphCacheInfo");
        return NULL;
    }

    gcinfo->head = NULL;
    gcinfo->tail = NULL;
    gcinfo->width = width;
    gcinfo->height = height;
    gcinfo->cellWidth = cellWidth;
    gcinfo->cellHeight = cellHeight;
    gcinfo->isFull = JNI_FALSE;

    return gcinfo;
}

/**
 * Attempts to add the provided glyph to the specified cache.  If the
 * operation is successful, a pointer to the newly occupied cache cell is
 * stored in the glyph's cellInfo field; otherwise, its cellInfo field is
 * set to NULL, indicating that the glyph's original bits should be rendered
 * instead.  If the cache is full, the least-recently-used glyph is
 * invalidated and its cache cell is reassigned to the new glyph being added.
 *
 * Note that this method only ensures that a rectangular region in the
 * "virtual" glyph cache is available for the glyph image.  Platform specific
 * glyph caching code is responsible for actually caching the glyph image
 * in the associated accelerated memory surface.
 */
void
AccelGlyphCache_AddGlyph(GlyphCacheInfo *cache, GlyphInfo *glyph)
{
    CacheCellInfo *cellinfo = NULL;
    jint w = glyph->width;
    jint h = glyph->height;

    J2dTraceLn(J2D_TRACE_INFO, "in AccelGlyphCache_AddGlyph");

    if ((glyph->width > cache->cellWidth) ||
        (glyph->height > cache->cellHeight))
    {
        return;
    }

    if (!cache->isFull) {
        jint x, y;

        if (cache->head == NULL) {
            x = 0;
            y = 0;
        } else {
            x = cache->tail->x + cache->cellWidth;
            y = cache->tail->y;
            if ((x + cache->cellWidth) > cache->width) {
                x = 0;
                y += cache->cellHeight;
                if ((y + cache->cellHeight) > cache->height) {
                    // no room left for a new cell; we'll go through the
                    // isFull path below
                    cache->isFull = JNI_TRUE;
                }
            }
        }

        if (!cache->isFull) {
            // create new CacheCellInfo
            cellinfo = (CacheCellInfo *)malloc(sizeof(CacheCellInfo));
            if (cellinfo == NULL) {
                glyph->cellInfo = NULL;
                J2dTraceLn(J2D_TRACE_ERROR, "could not allocate CellInfo");
                return;
            }

            cellinfo->cacheInfo = cache;
            cellinfo->glyphInfo = glyph;
            cellinfo->timesRendered = 0;
            cellinfo->x = x;
            cellinfo->y = y;
            cellinfo->tx1 = (jfloat)cellinfo->x / cache->width;
            cellinfo->ty1 = (jfloat)cellinfo->y / cache->height;
            cellinfo->tx2 = cellinfo->tx1 + ((jfloat)w / cache->width);
            cellinfo->ty2 = cellinfo->ty1 + ((jfloat)h / cache->height);

            if (cache->head == NULL) {
                // initialize the head cell
                cache->head = cellinfo;
            } else {
                // update existing tail cell
                cache->tail->next = cellinfo;
            }

            // add the new cell to the end of the list
            cache->tail = cellinfo;
            cellinfo->next = NULL;
        }
    }

    if (cache->isFull) {
        /**
         * Search through the cells, and for each cell:
         *   - reset its timesRendered counter to zero
         *   - toss it to the end of the list
         * Eventually we will find a cell that either:
         *   - is empty, or
         *   - has been used less than the threshold
         * When we find such a cell, we will:
         *   - break out of the loop
         *   - invalidate any glyph that may be residing in that cell
         *   - update the cell with the new resident glyph's information
         *
         * The goal here is to keep the glyphs rendered most often in the
         * cache, while younger glyphs hang out near the end of the list.
         * Those young glyphs that have only been used a few times will move
         * towards the head of the list and will eventually be kicked to
         * the curb.
         *
         * In the worst-case scenario, all cells will be occupied and they
         * will all have timesRendered counts above the threshold, so we will
         * end up iterating through all the cells exactly once.  Since we are
         * resetting their counters along the way, we are guaranteed to
         * eventually hit the original "head" cell, whose counter is now zero.
         * This avoids the possibility of an infinite loop.
         */

        do {
            // the head cell will be updated on each iteration
            CacheCellInfo *current = cache->head;

            if ((current->glyphInfo == NULL) ||
                (current->timesRendered < TIMES_RENDERED_THRESHOLD))
            {
                // all bow before the chosen one (we will break out of the
                // loop now that we've found an appropriate cell)
                cellinfo = current;
            }

            // move cell to the end of the list; update existing head and
            // tail pointers
            cache->head = current->next;
            cache->tail->next = current;
            cache->tail = current;
            current->next = NULL;
            current->timesRendered = 0;
        } while (cellinfo == NULL);

        if (cellinfo->glyphInfo != NULL) {
            // if the cell is occupied, notify the base glyph that its
            // cached version is about to be kicked out
            cellinfo->glyphInfo->cellInfo = NULL;
        }

        // update cellinfo with glyph's occupied region information
        cellinfo->glyphInfo = glyph;
        cellinfo->tx2 = cellinfo->tx1 + ((jfloat)w / cache->width);
        cellinfo->ty2 = cellinfo->ty1 + ((jfloat)h / cache->height);
    }

    // update the glyph's reference to its cache cell
    glyph->cellInfo = cellinfo;
}
