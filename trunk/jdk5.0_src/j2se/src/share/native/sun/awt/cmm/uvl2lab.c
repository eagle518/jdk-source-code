/*
 * @(#)uvl2lab.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	uvl2lab.c
 
	Contains:	This module contains functions to create uvL->Lab FuTs.

	From Poe via GBP, 2 Aug 00

	COPYRIGHT (c) 2000-2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcptmgr.h"
#include "makefuts.h"

#define	XWHITE	0.96819				/* Prophecy uvL = [87 196 255] */
#define YWHITE	1.00000
#define	ZWHITE	0.82830

#define	MID12BIT	0.50012210012210
					/* midpoint of 12-bit range:  2048/4095 */


/*--------------------------------------------------------------------------------
 *  Definitions for uv-mapping
 *--------------------------------------------------------------------------------
 */
#define U0	0.34117647058824 	/*  87/255; approximately, D50 */
#define V0	0.76862745098039	/* 196/255;       "           */

#define UGRID0	0.32258064516129	/* 10/31; neutral grid point */
#define	VGRID0	0.51612903225806	/* 16/31          "         */

#define	REMAP(x, a)	( (a) * (x) )
#define UNMAP(x, a)	( (x) / (a) )

/*--------------------------------------------------------------------
 * uvLLabInit -- determine slopes for piece-wise linear mapping
 *--------------------------------------------------------------------
 */
void
	uvLLabInit (uvLLabConst_p c)
{
	c->bu0 = (UGRID0 - 0.0) / (U0 - 0.0);
	c->bu1 = (1.0 - UGRID0) / (1.0 - U0);
	c->bv0 = (VGRID0 - 0.0) / (V0 - 0.0);
	c->bv1 = (1.0 - VGRID0) / (1.0 - V0);
}

/*---------------------------------------------------------------------
 *  xfun, yfun, zfun -- input mappings
 *---------------------------------------------------------------------
 */
double
	uvLLab_iu (double u, fut_calcData_p dataP)			/* piecewise linear in u' */
{
double	delta, a;

#if defined KCP_MAP_BASE_MAX_REF
	u *= KCP_16_TO_8_ENCODING;
#endif
	delta = u - U0;
	a = (delta > 0.0) ? ((auxData_p)dataP)->uvLLabC.bu1 : ((auxData_p)dataP)->uvLLabC.bu0;
	delta = REMAP (delta, a);
	u = UGRID0 + delta;

	return RESTRICT (u, 0.0, 1.0);
}

double uvLLab_iv (double v, fut_calcData_p dataP)			/* piecewise linear in v' */
{
double	delta, a;

#if defined KCP_MAP_BASE_MAX_REF
	v *= KCP_16_TO_8_ENCODING;
#endif
	delta = v - V0;
	a = (delta > 0.0) ? ((auxData_p)dataP)->uvLLabC.bv1 : ((auxData_p)dataP)->uvLLabC.bv0;
	delta = REMAP (delta, a);
	v = VGRID0 + delta;

	return RESTRICT (v, 0.0, 1.0);
}

double uvLLab_iL (double l, fut_calcData_p dataP)			/* piecewise linear in v' */
{
double	y;

#if defined KCP_MAP_BASE_MAX_REF
	l *= KCP_16_TO_8_ENCODING;
#endif
    /* Convert to luminance:  */
	y = Hinverse (l, &(((auxData_p)dataP)->lc));	/* ==> y is compressed relative luminance */

	y = (255.0 * y - 1.0) / 254.0;	/* decompress for L*a*b* */

    /* Convert back to (L*)/100:  */
	l = Hfunc (y, &(((auxData_p)dataP)->lc));				/* (L*)/100, in [-0.0356, 1] */

	return RESTRICT (l, 0.0, 1.0);
}

/*---------------------------------------------------------------------
 *  uvLLab_gFun -- grid-table functions
 *---------------------------------------------------------------------
 */
double uvLLab_gFun (double_p dP, fut_calcData_p dataP)
{
double	u = dP[0], v = dP[1], l = dP[2];
double	x, y, z, p, delta, a, astar, bstar;

     /* Decode piecewise linear functions of u and v:  */
	delta = u - UGRID0;
	a = (delta > 0.0) ? ((auxData_p)dataP)->uvLLabC.bu1 : ((auxData_p)dataP)->uvLLabC.bu0;
	delta = UNMAP (delta, a);
	u = U0 + delta;

	delta = v - VGRID0;
	a = (delta > 0.0) ? ((auxData_p)dataP)->uvLLabC.bv1 : ((auxData_p)dataP)->uvLLabC.bv0;
	delta = UNMAP (delta, a);
	v = V0 + delta;

	u = 0.070 + 0.40996784565916 * u;			/* CIE 1976 u' */
	v = 0.165 + 0.41986827661910 * v;			/* CIE 1976 v' */

     /* Compute CIE 1931 tristimulus values:  */
	y = Hinverse (l, &(((auxData_p)dataP)->lc));	/* CIE 1931 Y */
	y = (254.0 * y + 1.0) / 255.0;		/* recompress for Prophecy */
	x = 2.25 * (u / v) * y;					/* CIE 1931 X */
	z = ((3.0 - 0.75 * u) / v - 5.0) * y;			/* CIE 1931 Z */

     /* Scale to white point:  */
	x /= XWHITE;
	y /= YWHITE;
	z /= ZWHITE;

     /* Convert to CIELAB:  */
	astar = (Hfunc (x, &(((auxData_p)dataP)->lc)) - Hfunc (y, &(((auxData_p)dataP)->lc))) / 0.00232;	/* CIE 1976 a* */
	bstar = (Hfunc (y, &(((auxData_p)dataP)->lc)) - Hfunc (z, &(((auxData_p)dataP)->lc))) / 0.00580;	/* CIE 1976 b* */

     /* Encode CIELAB for L* in [0, 100] and a* & b* in [-200, 200]:  */
	switch (dataP->chan) {
	case 0:	/* (L*)/100 */
		p = Hfunc (y, &(((auxData_p)dataP)->lc));
		break;
	case 1:	/* ~ 1/2 + (a*)/400 */
		p = MID12BIT + 0.0025 * astar;
		break;
	case 2:	/* ~ 1/2 + (b*)/400 */
		p = MID12BIT + 0.0025 * bstar;
		break;
	default:	
		p = 6.023e+23;	/* Avogadro's number */
		break;
	}

	return RESTRICT (p, 0.0, 1.0);	
}

/*----------------------------------------------------------------------------
 *  outfun -- rescale and clip a* and b*; decompress L*
 *----------------------------------------------------------------------------
 */
double uvLLab_oFun (double p, fut_calcData_p dataP)
{
	switch (dataP->chan) {
	case 0:
		p = Hinverse (p, &(((auxData_p)dataP)->lc));	/* Y */
		p = (255.0 * p - 1.0) / 254.0;	/* decompress from Prophecy Dmax */
		p = Hfunc (p, &(((auxData_p)dataP)->lc));		/* (L*)/100, in [-0.0356, 1] */
		break;
	case 1:
	case 2:	p = 400.0 * (p - MID12BIT);	/* CIE 1976 a*, b*, in [-200, 200] */
		p = RESTRICT (p, -128.0, 127.0);	/* clip to [-128, 127] */
		p = p + 128.0;		/* -> [0, 255] */
		p /= 255.0;		/* from [0, 255] to [0, 1] */
		break;
	default:
		p = 6.023e+23;
		break;
	}

#if defined KCP_MAP_BASE_MAX_REF
	p *= KCP_8_TO_16_ENCODING;
#endif
	return RESTRICT (p, 0.0, 1.0); /* L* in [0, 100]; a* & b* in [-128, 127] */
}


