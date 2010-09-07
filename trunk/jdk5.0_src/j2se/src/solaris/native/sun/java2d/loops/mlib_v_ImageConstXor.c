/*
 * @(#)mlib_v_ImageConstXor.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#pragma ident	"@(#)mlib_v_ImageConstXor.c	1.1	03/04/29 SMI"

/*
 * FUNCTION
 *      mlib_ImageConstXor - image logical operation with constant
 *
 * SYNOPSIS
 *      mlib_status mlib_ImageConstXor(mlib_image       *dst,
 *                                     const mlib_image *src,
 *                                     const mlib_s32   *c);
 *
 * ARGUMENT
 *      dst     Pointer to destination image
 *      src     Pointer to source image
 *      c       Array of constants for each channel
 *
 * RESTRICTION
 *      The src and dst must be the same type and the same size.
 *      They can have 1, 2, 3, or 4 channels.
 *      They can be in MLIB_BIT, MLIB_BYTE, MLIB_SHORT, MLIB_USHORT or MLIB_INT
 *      data type.
 *
 * DESCRIPTION
 *      File for one of the following operations:
 *
 *      And  dst(i,j) = c & src(i,j)
 *      Or  dst(i,j) = c | src(i,j)
 *      Xor  dst(i,j) = c ^ src(i,j)
 *      NotAnd  dst(i,j) = ~(c & src(i,j))
 *      NotOr  dst(i,j) = ~(c | src(i,j))
 *      NotXor  dst(i,j) = ~(c ^ src(i,j))
 *      AndNot  dst(i,j) = c & (~src(i,j))
 *      OrNot  dst(i,j) = c & (~src(i,j))
 */

#include <mlib_image.h>

/***************************************************************/

#if ! defined ( __MEDIALIB_OLD_NAMES )
#if defined ( __SUNPRO_C )

#pragma weak mlib_ImageConstXor = __mlib_ImageConstXor

#elif defined ( __GNUC__ ) /* defined ( __SUNPRO_C ) */
  __typeof__ (__mlib_ImageConstXor) mlib_ImageConstXor
    __attribute__ ((weak,alias("__mlib_ImageConstXor")));

#else /* defined ( __SUNPRO_C ) */

#error  "unknown platform"

#endif /* defined ( __SUNPRO_C ) */
#endif /* ! defined ( __MEDIALIB_OLD_NAMES ) */

/***************************************************************/

#define VIS_CONSTLOGIC(c, a) vis_fxor(a, c)

#include <mlib_v_ImageConstLogic.h>

/***************************************************************/

mlib_status __mlib_ImageConstXor(mlib_image *dst,
                                 mlib_image *src,
                                 mlib_s32   *c)
{
  return mlib_v_ImageConstLogic(dst, src, c);
}

/***************************************************************/
