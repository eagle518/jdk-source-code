/*
 * @(#)mlib_ImageAffine_NN_Bit.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#ifdef __SUNPRO_C
#pragma ident	"@(#)mlib_ImageAffine_NN_Bit.c	1.4	02/05/08 SMI"
#endif /* __SUNPRO_C */

/*
 * FUNCTION
 *      Internal functions for mlib_ImageAffine with Nearest Neighbor filtering.
 */

#include "mlib_ImageAffine.h"

/***************************************************************/
#define DECLAREVAR_BIT()                                        \
  DECLAREVAR0();                                                \
  mlib_s32 ySrc;                                                \
  DTYPE *srcPixelPtr;                                           \
  DTYPE *srcPixelPtr0;                                          \
  DTYPE *srcPixelPtr1;                                          \
  DTYPE *srcPixelPtr2;                                          \
  DTYPE *srcPixelPtr3;                                          \
  DTYPE *srcPixelPtr4;                                          \
  DTYPE *srcPixelPtr5;                                          \
  DTYPE *srcPixelPtr6;                                          \
  DTYPE *srcPixelPtr7

/***************************************************************/
#define CLIP_BIT()                                              \
  dstData += dstYStride;                                        \
  xLeft  = leftEdges[j]  + d_bitoff;                            \
  xRight = rightEdges[j] + d_bitoff;                            \
  X = xStarts[j] + (s_bitoff << MLIB_SHIFT);                    \
  Y = yStarts[j];                                               \
  if (xLeft > xRight) continue

/***************************************************************/
#define DTYPE mlib_u8

void mlib_ImageAffine_bit_1ch_nn(mlib_affine_param *param,
                                 mlib_s32          s_bitoff,
                                 mlib_s32          d_bitoff)
{
  DECLAREVAR_BIT();
  mlib_s32 i, bit, res;

  for (j = yStart; j <= yFinish; j++) {

    CLIP_BIT();
    xRight++;

    i = xLeft;

    if (i & 7) {
      mlib_u8 *dp = dstData + (i >> 3);
      mlib_s32 res = dp[0];
      mlib_s32 i_end = i + (8 - (i & 7));

      if (i_end > xRight)
        i_end = xRight;

      for (; i < i_end; i++) {
        bit = 7 - (i & 7);
        ySrc = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);

        res = (res & ~(1 << bit)) | (((srcPixelPtr[X >> (MLIB_SHIFT + 3)] >> (7 - (X >> MLIB_SHIFT) & 7)) & 1) <<
           bit);

        X += dX;
        Y += dY;
      }

      dp[0] = res;
    }

#ifdef __SUNPRO_C
#pragma pipeloop(0)
#endif /* __SUNPRO_C */
    for (; i <= (xRight - 8); i += 8) {
      srcPixelPtr0 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res = ((srcPixelPtr0[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT)) & 7)) & 0x0080);
      X += dX;

      srcPixelPtr1 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr1[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 1) & 7)) & 0x4040);
      X += dX;

      srcPixelPtr2 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr2[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 2) & 7)) & 0x2020);
      X += dX;

      srcPixelPtr3 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr3[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 3) & 7)) & 0x1010);
      X += dX;

      srcPixelPtr4 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr4[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 4) & 7)) & 0x0808);
      X += dX;

      srcPixelPtr5 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr5[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 5) & 7)) & 0x0404);
      X += dX;

      srcPixelPtr6 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr6[X >> (MLIB_SHIFT + 3)] << (((X >> MLIB_SHIFT) - 6) & 7)) & 0x0202);
      X += dX;

      srcPixelPtr7 = MLIB_POINTER_GET(lineAddr, MLIB_POINTER_SHIFT(Y));
      Y += dY;
      res |= ((srcPixelPtr7[X >> (MLIB_SHIFT + 3)] >> (7 - ((X >> MLIB_SHIFT) & 7))) & 0x0001);
      X += dX;

      dstData[i >> 3] = res | (res >> 8);
    }

    if (i < xRight) {
      mlib_u8 *dp = dstData + (i >> 3);
      mlib_s32 res = dp[0];

      for (; i < xRight; i++) {
        bit = 7 - (i & 7);
        ySrc = MLIB_POINTER_SHIFT(Y);
        srcPixelPtr = MLIB_POINTER_GET(lineAddr, ySrc);

        res = (res & ~(1 << bit)) | (((srcPixelPtr[X >> (MLIB_SHIFT + 3)] >> (7 - (X >> MLIB_SHIFT) & 7)) & 1) << bit);

        X += dX;
        Y += dY;
      }

      dp[0] = res;
    }
  }
}

/***************************************************************/
