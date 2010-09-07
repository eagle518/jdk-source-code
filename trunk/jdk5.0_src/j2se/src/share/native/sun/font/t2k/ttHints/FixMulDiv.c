/*
 * @(#)FixMulDiv.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	Copyright ©1987-1993 Apple Computer, Inc.  All rights reserved.
*/
#include "Hint.h"
#include "FixMulDiv.h"


static void WideMul(tt_int32 src1, tt_int32 src2, tt_int32 dst[2])
{
	int negative = (src1 ^ src2) < 0;
	register tt_uint32 dsthi, dstlo;  

	if (src1 < 0)
		src1 = -src1;
	if (src2 < 0)
		src2 = -src2;
	{	unsigned short src1hi, src1lo;
		register unsigned short src2hi, src2lo;
		register tt_uint32 temp;
		src1hi = (unsigned short) (src1 >> 16);
		src1lo = (unsigned short)  src1;
		src2hi = (unsigned short) (src2 >> 16);
		src2lo = (unsigned short) (src2);
		temp =   (tt_uint32)src1hi * src2lo + (tt_uint32)src1lo * src2hi;
		dsthi = (tt_uint32)src1hi * src2hi + (temp >> 16);
		dstlo = (tt_uint32)src1lo * src2lo;
		temp <<= 16;
		dsthi += (dstlo += temp) < temp;
		dst[0] = dsthi;
		dst[1] = dstlo;
	}
	if (negative)
	{	
		/* fixed warning about unwanted assignment. */
		dstlo = (tt_uint32) (-(tt_int32)dstlo);
		if (dstlo )
			dsthi = ~dsthi;
		else
			dsthi = (tt_uint32) (-(tt_int32)dsthi);
	}
	dst[0] = dsthi;
	dst[1] = dstlo;
}

static tt_int32 WideDiv(tt_int32 src1, tt_int32 src2[2])
{
	register tt_uint32 src2hi = src2[0], src2lo = src2[1];
	int negative = (tt_int32)(src2hi ^ src1) < 0;

	if ((tt_int32)src2hi < 0)
	  {
		/* fixed warning about unwanted assignment. */
		src2lo = (tt_uint32) (-(tt_int32)src2lo);
		if (src2lo )
			src2hi = ~src2hi;
		else
			src2hi = (tt_uint32) (-(tt_int32)src2hi);
	  }
	if (src1 < 0)
		src1 = -src1;
	{	register tt_uint32 src1hi, src1lo;
		tt_uint32 result = 0, place = 0x40000000;

		if ((src1hi = src1) & 1)
			src1lo = 0x80000000;
		else
			src1lo = 0;

		src1hi >>= 1;
		src2hi += (src2lo += src1hi) < src1hi;		/* round the result */

		if (src2hi > src1hi || src2hi == src1hi && src2lo >= src1lo)
			if (negative)
				return NEGINFINITY;
			else
				return POSINFINITY;
		while (place && src2hi)
		{	src1lo >>= 1;
			if (src1hi & 1)
				src1lo += 0x80000000;
			src1hi >>= 1;
			if (src1hi < src2hi)
			{	src2hi -= src1hi;
				src2hi -= src1lo > src2lo;
				src2lo -= src1lo;
				result += place;
			}
			else if (src1hi == src2hi && src1lo <= src2lo)
			{	src2hi = 0;
				src2lo -= src1lo;
				result += place;
			}
			place >>= 1;
		}
		if (src2lo >= (tt_uint32) src1)
			result += src2lo/src1;
		if (negative)
			return -(tt_int32)result;
		else
			return result;
	}
}

/*
 *	a*b/c
 */
tt_int32 MultiplyDivide(tt_int32 a, tt_int32 b, tt_int32 c)
{
	tt_int32 temp[2];

	WideMul(a, b, temp);
	return WideDiv(c, temp);
}

 
tt_int32 MultiplyFract(tt_int32 a, tt_int32 b)
{
	tt_int32 temp[2];
	tt_int32 resLo;
	tt_uint32 resHi;
	
	/* first perform a full multiply */
	WideMul(a, b, temp);
	resHi= (tt_int32) temp[0];
	resLo=(tt_uint32) temp[1];
	return( (resHi << 2) +  ( resLo>>(32-2) ) );
}
