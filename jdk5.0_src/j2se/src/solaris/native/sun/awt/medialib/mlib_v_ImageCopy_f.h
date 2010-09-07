/*
 * @(#)mlib_v_ImageCopy_f.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_V_IMAGECOPY_F_H
#define __MLIB_V_IMAGECOPY_F_H

#pragma ident	"@(#)mlib_v_ImageCopy_f.h	1.3	02/07/23 SMI"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void mlib_v_ImageCopy_a1(mlib_d64 *sp,
                         mlib_d64 *dp,
                         mlib_s32 size);

void mlib_v_ImageCopy_a2(mlib_d64 *sp,
                         mlib_d64 *dp,
                         mlib_s32 width,
                         mlib_s32 height,
                         mlib_s32 stride,
                         mlib_s32 dstride);

void mlib_v_ImageCopy_blk(const void *src,
                          void       *dst,
                          mlib_s32   size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_V_IMAGECOPY_F_H */
