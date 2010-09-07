/*
 * @(#)mlib_c_ImageThresh1.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_C_IMAGETHRESH1_H
#define __MLIB_C_IMAGETHRESH1_H

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_c_ImageThresh1.h	1.1	02/03/28 SMI"
#endif /* __SUNPRO_C */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************/
#define PARAMS                                                  \
  void     *psrc,                                               \
  void     *pdst,                                               \
  mlib_s32 src_stride,                                          \
  mlib_s32 dst_stride,                                          \
  mlib_s32 width,                                               \
  mlib_s32 height,                                              \
  void     *__thresh,                                           \
  void     *__ghigh,                                            \
  void     *__glow

void mlib_c_ImageThresh1_D641(PARAMS);
void mlib_c_ImageThresh1_D642(PARAMS);
void mlib_c_ImageThresh1_D643(PARAMS);
void mlib_c_ImageThresh1_D644(PARAMS);
void mlib_c_ImageThresh1_D641_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_D642_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_D643_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_D644_1B(PARAMS, mlib_s32 dbit_off);

void mlib_c_ImageThresh1_F321(PARAMS);
void mlib_c_ImageThresh1_F322(PARAMS);
void mlib_c_ImageThresh1_F323(PARAMS);
void mlib_c_ImageThresh1_F324(PARAMS);
void mlib_c_ImageThresh1_F321_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_F322_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_F323_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_F324_1B(PARAMS, mlib_s32 dbit_off);

void mlib_c_ImageThresh1_S321(PARAMS);
void mlib_c_ImageThresh1_S322(PARAMS);
void mlib_c_ImageThresh1_S323(PARAMS);
void mlib_c_ImageThresh1_S324(PARAMS);
void mlib_c_ImageThresh1_S321_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S322_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S323_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S324_1B(PARAMS, mlib_s32 dbit_off);

void mlib_c_ImageThresh1_S161(PARAMS);
void mlib_c_ImageThresh1_S162(PARAMS);
void mlib_c_ImageThresh1_S163(PARAMS);
void mlib_c_ImageThresh1_S164(PARAMS);
void mlib_c_ImageThresh1_S161_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S162_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S163_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_S164_1B(PARAMS, mlib_s32 dbit_off);

void mlib_c_ImageThresh1_U161(PARAMS);
void mlib_c_ImageThresh1_U162(PARAMS);
void mlib_c_ImageThresh1_U163(PARAMS);
void mlib_c_ImageThresh1_U164(PARAMS);
void mlib_c_ImageThresh1_U161_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U162_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U163_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U164_1B(PARAMS, mlib_s32 dbit_off);

void mlib_c_ImageThresh1_U81(PARAMS);
void mlib_c_ImageThresh1_U82(PARAMS);
void mlib_c_ImageThresh1_U83(PARAMS);
void mlib_c_ImageThresh1_U84(PARAMS);
void mlib_c_ImageThresh1_U81_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U82_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U83_1B(PARAMS, mlib_s32 dbit_off);
void mlib_c_ImageThresh1_U84_1B(PARAMS, mlib_s32 dbit_off);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_C_IMAGETHRESH1_H */
