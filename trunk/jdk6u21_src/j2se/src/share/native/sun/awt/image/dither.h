/*
 * @(#)dither.h	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "colordata.h"

#ifdef __cplusplus
extern "C" {
#endif

extern sgn_ordered_dither_array std_img_oda_red;
extern sgn_ordered_dither_array std_img_oda_green;
extern sgn_ordered_dither_array std_img_oda_blue;
extern int std_odas_computed;

void make_dither_arrays(int cmapsize, ColorData *cData);
void initInverseGrayLut(int* prgb, int rgbsize, ColorData* cData);

/*
 * state info needed for breadth-first recursion of color cube from
 * initial palette entries within the cube
 */

typedef struct {
    unsigned int depth;
    unsigned int maxDepth;

    unsigned char *usedFlags;
    unsigned int  activeEntries;
    unsigned short *rgb;
    unsigned char *indices;
    unsigned char *iLUT;
} CubeStateInfo;

#define INSERTNEW(state, rgb, index) do {                           \
        if (!state.usedFlags[rgb]) {                                \
            state.usedFlags[rgb] = 1;                               \
            state.iLUT[rgb] = index;                                \
            state.rgb[state.activeEntries] = rgb;                   \
            state.indices[state.activeEntries] = index;             \
            state.activeEntries++;                                  \
        }                                                           \
} while (0);
    

#define ACTIVATE(code, mask, delta, state, index) do {              \
    if (((rgb & mask) + delta) <= mask) {                           \
        rgb += delta;                                               \
        INSERTNEW(state, rgb, index);                               \
        rgb -= delta;                                               \
    }                                                               \
    if ((rgb & mask) >= delta) {                                    \
        rgb -= delta;                                               \
        INSERTNEW(state, rgb, index);                               \
        rgb += delta;                                               \
    }                                                               \
} while (0);

#ifdef __cplusplus
} /* extern "C" */
#endif
