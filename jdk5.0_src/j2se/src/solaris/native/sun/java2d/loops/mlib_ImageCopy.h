/*
 * @(#)mlib_ImageCopy.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MLIB_IMAGECOPY_H
#define __MLIB_IMAGECOPY_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageCopy.h	1.2	03/01/30 SMI"
#endif /* __SUNPRO_C */

#include <mlib_types.h>
#include <mlib_status.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void mlib_ImageCopy_bit_al(const mlib_u8 *sa,
                           mlib_u8       *da,
                           mlib_s32      size,
                           mlib_s32      offset);

void mlib_ImageCopy_na(const mlib_u8 *sp,
                       mlib_u8       *dp,
                       mlib_s32      n);

void mlib_ImageCopy_bit_na_r(const mlib_u8 *sa,
                             mlib_u8       *da,
                             mlib_s32      size,
                             mlib_s32      s_offset,
                             mlib_s32      d_offset);

void mlib_ImageCopy_bit_na(const mlib_u8 *sa,
                           mlib_u8       *da,
                           mlib_s32      size,
                           mlib_s32      s_offset,
                           mlib_s32      d_offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_IMAGECOPY_H */
