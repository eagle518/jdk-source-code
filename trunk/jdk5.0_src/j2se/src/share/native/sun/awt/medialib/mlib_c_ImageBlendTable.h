/*
 * @(#)mlib_c_ImageBlendTable.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
#ifdef __SUNPRO_C
#pragma ident   "@(#)mlib_c_ImageBlendTable.h	1.4    99/07/29 SMI"
#endif /* __SUNPRO_C */

/*
 *    These tables are used by C versions of the
 *    mlib_ImageBlend_... functions.
 */

#ifndef MLIB_C_IMAGE_BLEND_TABLE_H
#define MLIB_C_IMAGE_BLEND_TABLE_H

#include "mlib_image.h"

extern const mlib_f32 mlib_c_blend_u8[];
extern const mlib_f32 mlib_U82F32[];
extern const mlib_f32 mlib_c_blend_Q8[];
extern const mlib_f32 mlib_c_blend_u8_sat[];

#endif /* MLIB_C_IMAGEF_BLEND_TABLE_H */

