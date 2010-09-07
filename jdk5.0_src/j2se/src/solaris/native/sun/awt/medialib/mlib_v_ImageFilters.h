/*
 * @(#)mlib_v_ImageFilters.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_V_IMAGEFILTERS_H
#define __MLIB_V_IMAGEFILTERS_H

#pragma ident	"@(#)mlib_v_ImageFilters.h	1.19	02/05/08 SMI"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *    These tables are used by VIS versions
 *    of the following functions:
 *      mlib_ImageRotate(Index)
 *      mlib_ImageAffine(Index)
 *      mlib_ImageZoom(Index)
 *      mlib_ImageGridWarp
 *      mlib_ImagePolynomialWarp
 */

#include "mlib_image.h"

#if defined (__INIT_TABLE)

#pragma align 8 (mlib_filters_u8_bl)
#pragma align 8 (mlib_filters_u8_bc)
#pragma align 8 (mlib_filters_u8_bc2)
#pragma align 8 (mlib_filters_u8_bc_3)
#pragma align 8 (mlib_filters_u8_bc2_3)
#pragma align 8 (mlib_filters_u8_bc_4)
#pragma align 8 (mlib_filters_u8_bc2_4)
#pragma align 8 (mlib_filters_s16_bc)
#pragma align 8 (mlib_filters_s16_bc2)
#pragma align 8 (mlib_filters_s16_bc_3)
#pragma align 8 (mlib_filters_s16_bc2_3)
#pragma align 8 (mlib_filters_s16_bc_4)
#pragma align 8 (mlib_filters_s16_bc2_4)

#endif /* defined (__INIT_TABLE) */

extern const mlib_s16 mlib_filters_u8_bl[];
extern const mlib_s16 mlib_filters_u8_bc[];
extern const mlib_s16 mlib_filters_u8_bc2[];
extern const mlib_s16 mlib_filters_u8_bc_3[];
extern const mlib_s16 mlib_filters_u8_bc2_3[];
extern const mlib_s16 mlib_filters_u8_bc_4[];
extern const mlib_s16 mlib_filters_u8_bc2_4[];
extern const mlib_s16 mlib_filters_s16_bc[];
extern const mlib_s16 mlib_filters_s16_bc2[];
extern const mlib_s16 mlib_filters_s16_bc_3[];
extern const mlib_s16 mlib_filters_s16_bc2_3[];
extern const mlib_s16 mlib_filters_s16_bc_4[];
extern const mlib_s16 mlib_filters_s16_bc2_4[];

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_V_IMAGEFILTERS_H */
