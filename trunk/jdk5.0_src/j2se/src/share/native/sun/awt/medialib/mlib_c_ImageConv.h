/*
 * @(#)mlib_c_ImageConv.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_C_IMAGECONV_H
#define __MLIB_C_IMAGECONV_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageConv.h	1.1	02/04/19 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

mlib_status mlib_c_conv2x2ext_s16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_c_conv2x2ext_u16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_c_conv2x2ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv2x2nw_s16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv2x2nw_u16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv2x2nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_conv3x3ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv3x3nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_conv4x4ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv4x4nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_conv5x5ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv5x5nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_conv7x7ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_c_conv7x7nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_convMxNnw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kernel,
                                mlib_s32         m,
                                mlib_s32         n,
                                mlib_s32         dm,
                                mlib_s32         dn,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_c_convMxNext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         m,
                                 mlib_s32         n,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

#if ! defined ( __sparc ) /* for x86, using integer multiplies is faster */

mlib_status mlib_i_conv3x3ext_s16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_i_conv3x3ext_u16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_i_conv3x3ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv3x3nw_s16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv3x3nw_u16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv3x3nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_i_conv5x5ext_s16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_i_conv5x5ext_u16(mlib_image       *dst,
                                  const mlib_image *src,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  const mlib_s32   *kern,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_i_conv5x5ext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv5x5nw_s16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv5x5nw_u16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_conv5x5nw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kern,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_i_convMxNnw_s16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kernel,
                                 mlib_s32         m,
                                 mlib_s32         n,
                                 mlib_s32         dm,
                                 mlib_s32         dn,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_convMxNnw_u16(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kernel,
                                 mlib_s32         m,
                                 mlib_s32         n,
                                 mlib_s32         dm,
                                 mlib_s32         dn,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_convMxNnw_u8(mlib_image       *dst,
                                const mlib_image *src,
                                const mlib_s32   *kernel,
                                mlib_s32         m,
                                mlib_s32         n,
                                mlib_s32         dm,
                                mlib_s32         dn,
                                mlib_s32         scale,
                                mlib_s32         cmask);

mlib_status mlib_i_convMxNext_u8(mlib_image       *dst,
                                 const mlib_image *src,
                                 const mlib_s32   *kern,
                                 mlib_s32         m,
                                 mlib_s32         n,
                                 mlib_s32         dx_l,
                                 mlib_s32         dx_r,
                                 mlib_s32         dy_t,
                                 mlib_s32         dy_b,
                                 mlib_s32         scale,
                                 mlib_s32         cmask);

mlib_status mlib_i_convMxNext_s16(mlib_image       *dst,
                                  const mlib_image *src,
                                  const mlib_s32   *kernel,
                                  mlib_s32         m,
                                  mlib_s32         n,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

mlib_status mlib_i_convMxNext_u16(mlib_image       *dst,
                                  const mlib_image *src,
                                  const mlib_s32   *kernel,
                                  mlib_s32         m,
                                  mlib_s32         n,
                                  mlib_s32         dx_l,
                                  mlib_s32         dx_r,
                                  mlib_s32         dy_t,
                                  mlib_s32         dy_b,
                                  mlib_s32         scale,
                                  mlib_s32         cmask);

#endif /* ! defined ( __sparc ) ( for x86, using integer multiplies is faster ) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_C_IMAGECONV_H */
