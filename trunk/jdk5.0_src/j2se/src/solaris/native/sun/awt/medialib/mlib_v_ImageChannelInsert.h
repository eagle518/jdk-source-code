/*
 * @(#)mlib_v_ImageChannelInsert.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifndef __MLIB_V_IMAGECHANNELINSERT_H
#define __MLIB_V_IMAGECHANNELINSERT_H

#pragma ident	"@(#)mlib_v_ImageChannelInsert.h	1.2	02/07/23 SMI"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void mlib_v_ImageChannelInsert_U8(const mlib_u8 *src,
                                  mlib_s32      slb,
                                  mlib_u8       *dst,
                                  mlib_s32      dlb,
                                  mlib_s32      channels,
                                  mlib_s32      channeld,
                                  mlib_s32      width,
                                  mlib_s32      height,
                                  mlib_s32      cmask);

void mlib_v_ImageChannelInsert_D64(const mlib_d64 *src,
                                   mlib_s32       slb,
                                   mlib_d64       *dst,
                                   mlib_s32       dlb,
                                   mlib_s32       channels,
                                   mlib_s32       channeld,
                                   mlib_s32       width,
                                   mlib_s32       height,
                                   mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16(const mlib_s16 *src,
                                   mlib_s32       slb,
                                   mlib_s16       *dst,
                                   mlib_s32       dlb,
                                   mlib_s32       channels,
                                   mlib_s32       channeld,
                                   mlib_s32       width,
                                   mlib_s32       height,
                                   mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S32(const mlib_s32 *src,
                                   mlib_s32       slb,
                                   mlib_s32       *dst,
                                   mlib_s32       dlb,
                                   mlib_s32       channels,
                                   mlib_s32       channeld,
                                   mlib_s32       width,
                                   mlib_s32       height,
                                   mlib_s32       cmask);

void mlib_v_ImageChannelInsert_U8_12_A8D1X8(const mlib_u8 *src,
                                            mlib_u8       *dst,
                                            mlib_s32      dsize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_12_A8D2X8(const mlib_u8 *src,
                                            mlib_s32      slb,
                                            mlib_u8       *dst,
                                            mlib_s32      dlb,
                                            mlib_s32      xsize,
                                            mlib_s32      ysize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_12_D1(const mlib_u8 *src,
                                        mlib_u8       *dst,
                                        mlib_s32      dsize,
                                        mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_12(const mlib_u8 *src,
                                     mlib_s32      slb,
                                     mlib_u8       *dst,
                                     mlib_s32      dlb,
                                     mlib_s32      xsize,
                                     mlib_s32      ysize,
                                     mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_13_A8D1X8(const mlib_u8 *src,
                                            mlib_u8       *dst,
                                            mlib_s32      dsize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_13_A8D2X8(const mlib_u8 *src,
                                            mlib_s32      slb,
                                            mlib_u8       *dst,
                                            mlib_s32      dlb,
                                            mlib_s32      xsize,
                                            mlib_s32      ysize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_13_D1(const mlib_u8 *src,
                                        mlib_u8       *dst,
                                        mlib_s32      dsize,
                                        mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_13(const mlib_u8 *src,
                                     mlib_s32      slb,
                                     mlib_u8       *dst,
                                     mlib_s32      dlb,
                                     mlib_s32      xsize,
                                     mlib_s32      ysize,
                                     mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_14_A8D1X8(const mlib_u8 *src,
                                            mlib_u8       *dst,
                                            mlib_s32      dsize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_14_A8D2X8(const mlib_u8 *src,
                                            mlib_s32      slb,
                                            mlib_u8       *dst,
                                            mlib_s32      dlb,
                                            mlib_s32      xsize,
                                            mlib_s32      ysize,
                                            mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_14_D1(const mlib_u8 *src,
                                        mlib_u8       *dst,
                                        mlib_s32      dsize,
                                        mlib_s32      cmask);

void mlib_v_ImageChannelInsert_U8_14(const mlib_u8 *src,
                                     mlib_s32      slb,
                                     mlib_u8       *dst,
                                     mlib_s32      dlb,
                                     mlib_s32      xsize,
                                     mlib_s32      ysize,
                                     mlib_s32      cmask);

void mlib_v_ImageChannelInsert_S16_12_A8D1X4(const mlib_s16 *src,
                                             mlib_s16       *dst,
                                             mlib_s32       dsize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_12_A8D2X4(const mlib_s16 *src,
                                             mlib_s32       slb,
                                             mlib_s16       *dst,
                                             mlib_s32       dlb,
                                             mlib_s32       xsize,
                                             mlib_s32       ysize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_12_D1(const mlib_s16 *src,
                                         mlib_s16       *dst,
                                         mlib_s32       dsize,
                                         mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_12(const mlib_s16 *src,
                                      mlib_s32       slb,
                                      mlib_s16       *dst,
                                      mlib_s32       dlb,
                                      mlib_s32       xsize,
                                      mlib_s32       ysize,
                                      mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_13_A8D1X4(const mlib_s16 *src,
                                             mlib_s16       *dst,
                                             mlib_s32       dsize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_13_A8D2X4(const mlib_s16 *src,
                                             mlib_s32       slb,
                                             mlib_s16       *dst,
                                             mlib_s32       dlb,
                                             mlib_s32       xsize,
                                             mlib_s32       ysize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_13_D1(const mlib_s16 *src,
                                         mlib_s16       *dst,
                                         mlib_s32       dsize,
                                         mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_13(const mlib_s16 *src,
                                      mlib_s32       slb,
                                      mlib_s16       *dst,
                                      mlib_s32       dlb,
                                      mlib_s32       xsize,
                                      mlib_s32       ysize,
                                      mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_14_A8D1X4(const mlib_s16 *src,
                                             mlib_s16       *dst,
                                             mlib_s32       dsize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_14_A8D2X4(const mlib_s16 *src,
                                             mlib_s32       slb,
                                             mlib_s16       *dst,
                                             mlib_s32       dlb,
                                             mlib_s32       xsize,
                                             mlib_s32       ysize,
                                             mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_14_D1(const mlib_s16 *src,
                                         mlib_s16       *dst,
                                         mlib_s32       dsize,
                                         mlib_s32       cmask);

void mlib_v_ImageChannelInsert_S16_14(const mlib_s16 *src,
                                      mlib_s32       slb,
                                      mlib_s16       *dst,
                                      mlib_s32       dlb,
                                      mlib_s32       xsize,
                                      mlib_s32       ysize,
                                      mlib_s32       cmask);

void mlib_v_ImageChannelInsert_U8_34R_A8D1X8(const mlib_u8 *src,
                                             mlib_u8       *dst,
                                             mlib_s32      dsize);

void mlib_v_ImageChannelInsert_U8_34R_A8D2X8(const mlib_u8 *src,
                                             mlib_s32      slb,
                                             mlib_u8       *dst,
                                             mlib_s32      dlb,
                                             mlib_s32      xsize,
                                             mlib_s32      ysize);

void mlib_v_ImageChannelInsert_U8_34R_D1(const mlib_u8 *src,
                                         mlib_u8       *dst,
                                         mlib_s32      dsize);

void mlib_v_ImageChannelInsert_U8_34R(const mlib_u8 *src,
                                      mlib_s32      slb,
                                      mlib_u8       *dst,
                                      mlib_s32      dlb,
                                      mlib_s32      xsize,
                                      mlib_s32      ysize);

void mlib_v_ImageChannelInsert_S16_34R_A8D1X4(const mlib_s16 *src,
                                              mlib_s16       *dst,
                                              mlib_s32       dsize);

void mlib_v_ImageChannelInsert_S16_34R_A8D2X4(const mlib_s16 *src,
                                              mlib_s32       slb,
                                              mlib_s16       *dst,
                                              mlib_s32       dlb,
                                              mlib_s32       xsize,
                                              mlib_s32       ysize);

void mlib_v_ImageChannelInsert_S16_34R_D1(const mlib_s16 *src,
                                          mlib_s16       *dst,
                                          mlib_s32       dsize);

void mlib_v_ImageChannelInsert_S16_34R(const mlib_s16 *src,
                                       mlib_s32       slb,
                                       mlib_s16       *dst,
                                       mlib_s32       dlb,
                                       mlib_s32       xsize,
                                       mlib_s32       ysize);

void mlib_v_ImageChannelInsert_U8_34L_A8D1X8(const mlib_u8 *src,
                                             mlib_u8       *dst,
                                             mlib_s32      dsize);

void mlib_v_ImageChannelInsert_U8_34L_A8D2X8(const mlib_u8 *src,
                                             mlib_s32      slb,
                                             mlib_u8       *dst,
                                             mlib_s32      dlb,
                                             mlib_s32      xsize,
                                             mlib_s32      ysize);

void mlib_v_ImageChannelInsert_U8_34L_D1(const mlib_u8 *src,
                                         mlib_u8       *dst,
                                         mlib_s32      dsize);

void mlib_v_ImageChannelInsert_U8_34L(const mlib_u8 *src,
                                      mlib_s32      slb,
                                      mlib_u8       *dst,
                                      mlib_s32      dlb,
                                      mlib_s32      xsize,
                                      mlib_s32      ysize);

void mlib_v_ImageChannelInsert_S16_34L_A8D1X4(const mlib_s16 *src,
                                              mlib_s16       *dst,
                                              mlib_s32       dsize);

void mlib_v_ImageChannelInsert_S16_34L_A8D2X4(const mlib_s16 *src,
                                              mlib_s32       slb,
                                              mlib_s16       *dst,
                                              mlib_s32       dlb,
                                              mlib_s32       xsize,
                                              mlib_s32       ysize);

void mlib_v_ImageChannelInsert_S16_34L_D1(const mlib_s16 *src,
                                          mlib_s16       *dst,
                                          mlib_s32       dsize);

void mlib_v_ImageChannelInsert_S16_34L(const mlib_s16 *src,
                                       mlib_s32       slb,
                                       mlib_s16       *dst,
                                       mlib_s32       dlb,
                                       mlib_s32       xsize,
                                       mlib_s32       ysize);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __MLIB_V_IMAGECHANNELINSERT_H */
