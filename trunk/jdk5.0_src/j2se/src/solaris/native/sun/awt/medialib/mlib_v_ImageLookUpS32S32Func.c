/*
 * @(#)mlib_v_ImageLookUpS32S32Func.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  

#pragma ident	"@(#)mlib_v_ImageLookUpS32S32Func.c	1.5	02/03/28 SMI"

#include "mlib_image.h"
#include "mlib_v_ImageLookUpFunc.h"

/***************************************************************/
#define HALF_U64        (MLIB_U64_CONST(2147483648) * sizeof(table[0][0]))

/***************************************************************/
void mlib_v_ImageLookUp_S32_S32(const mlib_s32 *src,
                                mlib_s32       slb,
                                mlib_s32       *dst,
                                mlib_s32       dlb,
                                mlib_s32       xsize,
                                mlib_s32       ysize,
                                const mlib_s32 **table,
                                mlib_s32       csize)
{
  mlib_s32 i, j, k;

  dlb >>= 2; slb >>= 2;

  if (xsize < 2) {
    for(j = 0; j < ysize; j++, dst += dlb, src += slb){
      for(k = 0; k < csize; k++) {
        mlib_s32 *da = dst + k;
        const mlib_s32 *sa = src + k;
        const mlib_s32 *tab = (void *)&(((mlib_u8 **)table)[k][HALF_U64]);

        for(i = 0; i < xsize; i++, da += csize, sa += csize)
        *da=tab[*sa];
      }
    }

  } else {
    for(j = 0; j < ysize; j++, dst += dlb, src += slb) {
#pragma pipeloop(0)
      for(k = 0; k < csize; k++) {
        mlib_s32 *da = dst + k;
        const mlib_s32 *sa = src + k;
        const mlib_s32 *tab = (void *)&(((mlib_u8 **)table)[k][HALF_U64]);
        mlib_s32 s0, t0, s1, t1;

        s0 = sa[0];
        s1 = sa[csize];
        sa += 2*csize;

        for(i = 0; i < xsize - 3; i+=2, da += 2*csize, sa += 2*csize) {
          t0 = tab[s0];
          t1 = tab[s1];
          s0 = sa[0];
          s1 = sa[csize];
          da[0] = t0;
          da[csize] = t1;
        }

        t0 = tab[s0];
        t1 = tab[s1];
        da[0] = t0;
        da[csize] = t1;

        if (xsize & 1) da[2*csize] = tab[sa[0]];
      }
    }
  }
}

/***************************************************************/
