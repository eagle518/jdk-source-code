/*
 * @(#)mlib_ImageClipping.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_IMAGECLIPPING_H
#define __MLIB_IMAGECLIPPING_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageClipping.h	1.1	02/04/19 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

mlib_status mlib_ImageClippingMxN(mlib_image *dst_i,
                                  mlib_image *src_i,
                                  mlib_image *dst_e,
                                  mlib_image *src_e,
                                  mlib_s32 *edg_sizes,
                                  const mlib_image *dst,
                                  const mlib_image *src,
                                  mlib_s32 kw,
                                  mlib_s32 kh,
                                  mlib_s32 kw1,
                                  mlib_s32 kh1);

mlib_status mlib_ImageClipping(mlib_image *dst_i,
                               mlib_image *src_i,
                               mlib_image *dst_e,
                               mlib_image *src_e,
                               mlib_s32 *edg_sizes,
                               const mlib_image *dst,
                               const mlib_image *src,
                               mlib_s32 ker_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_IMAGECLIPPING_H */
