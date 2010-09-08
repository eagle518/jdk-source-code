/*
 * @(#)ParallelogramUtils.h	1.2 10/03/23
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef ParallelogramUtils_h_Included
#define ParallelogramUtils_h_Included

#ifdef __cplusplus
extern "C" {
#endif

#define PGRAM_MIN_MAX(bmin, bmax, v0, dv1, dv2, AA) \
    do { \
        double vmin, vmax; \
        if (dv1 < 0) { \
            vmin = v0+dv1; \
            vmax = v0; \
        } else { \
            vmin = v0; \
            vmax = v0+dv1; \
        } \
        if (dv2 < 0) { \
            vmin += dv2; \
        } else { \
            vmax += dv2; \
        } \
        if (AA) { \
            bmin = (jint) floor(vmin); \
            bmax = (jint) ceil(vmax); \
        } else { \
            bmin = (jint) floor(vmin + 0.5); \
            bmax = (jint) floor(vmax + 0.5); \
        } \
    } while(0)

#define PGRAM_INIT_X(starty, x, y, slope) \
    (DblToLong((x) + (slope) * ((starty)+0.5 - (y))) + LongOneHalf - 1)

/*
 * Sort parallelogram by y values, ensure that each delta vector
 * has a non-negative y delta.
 */
#define SORT_PGRAM(x0, y0, dx1, dy1, dx2, dy2, OTHER_SWAP_CODE) \
    do { \
        if (dy1 < 0) { \
            x0 += dx1;  y0 += dy1; \
            dx1 = -dx1; dy1 = -dy1; \
        } \
        if (dy2 < 0) { \
            x0 += dx2;  y0 += dy2; \
            dx2 = -dx2; dy2 = -dy2; \
        } \
        /* Sort delta vectors so dxy1 is left of dxy2. */ \
        if (dx1 * dy2 > dx2 * dy1) { \
            double v; \
            v = dx1; dx1 = dx2; dx2 = v; \
            v = dy1; dy1 = dy2; dy2 = v; \
            OTHER_SWAP_CODE \
        } \
    } while(0)

#endif /* ParallelogramUtils_h_Included */
