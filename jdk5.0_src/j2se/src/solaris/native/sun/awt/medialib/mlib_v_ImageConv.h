/*
 * @(#)mlib_v_ImageConv.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_V_IMAGECONV_H
#define __MLIB_V_IMAGECONV_H

#pragma ident	"@(#)mlib_v_ImageConv.h	1.2	02/07/23 SMI"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined ( VIS ) && VIS == 0x200

mlib_status mlib_conv2x2_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               const mlib_s32   *kern,
                               mlib_s32         scale,
                               mlib_s32         cmask);

mlib_status mlib_conv3x3_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               const mlib_s32   *kern,
                               mlib_s32         scale,
                               mlib_s32         cmask);

mlib_status mlib_convMxN_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               mlib_s32         m,
                               mlib_s32         n,
                               mlib_s32         dm,
                               mlib_s32         dn,
                               const mlib_s32   *kern,
                               mlib_s32         scale,
                               mlib_s32         cmask);

#else

mlib_status mlib_conv2x2_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               const mlib_s32   *kern,
                               mlib_s32         scale);

mlib_status mlib_conv3x3_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               const mlib_s32   *kern,
                               mlib_s32         scale);

mlib_status mlib_convMxN_8nw_f(mlib_image       *dst,
                               const mlib_image *src,
                               mlib_s32         m,
                               mlib_s32         n,
                               mlib_s32         dm,
                               mlib_s32         dn,
                               const mlib_s32   *kern,
                               mlib_s32         scale);

#endif /* defined ( VIS ) && VIS == 0x200 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_V_IMAGECONV_H */
