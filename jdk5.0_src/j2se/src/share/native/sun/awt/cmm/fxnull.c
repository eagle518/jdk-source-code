/*
 * @(#)fxnull.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	fxnull.c

	Contains:	Functions used to create the fxnull auxiliary PT used for chaining

	COPYRIGHT (c) 1991-2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "makefuts.h"

#define U0		0.33725490196078		/* 86/255; approximately, D50 */
#define V0		0.76862745098039		/* 196/255;       "           */
#define UGRID0	0.33333333333333		/* 5/15; neutral grid point */
#define VGRID0	0.60000000000000		/* 9/15;          "         */
#define SLOPE	0.33333333333333		/* 1/3; slope at neutral point */

#define	AU0	-2.03529411764706		/* coefficient of quadratic term for low u  */
												/* = (U0/UGRID0 - SLOPE)/UGRID0 */
#define	AU1	0.99117647058823		/* coefficient of quadratic term for high u */
												/* = ((1.0 - U0)/(1.0 - UGRID0) - SLOPE)/
																(1.0 - UGRID0) */
#define AV0 -1.57952069716776		/* coefficient of quadratic term for low v  */
												/* = (V0/VGRID0 - SLOPE)/VGRID0 */
#define AV1	0.61274509803922			/* coefficient of quadratic term for high v */
												/* = ((1.0 - V0)/(1.0 - VGRID0) - SLOPE)/
																(1.0 - VGRID0) */


#define	REMAP(x,a)	( (sqrt (1.0 + 4.0 * (a) * (x) / (SLOPE * SLOPE)) - 1.0) \
								* (0.5 * SLOPE / (a)) )
#define	UNMAP(x,a)	( (SLOPE + (a) * (x)) * (x) )

/*---------------------------------------------------------------------
 *  fxnull_iFunc_x, fxnull_iFunc_y, fxnull_iFunc_z -- input mappings
 *---------------------------------------------------------------------
 */
double fxnull_iFunc_x
	(double u, fut_calcData_p dataP)			/* S-shaped curve in u' */
{
double	delta, a;

	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	u *= KCP_16_TO_8_ENCODING;
#endif
	delta = u - U0;
	a = (delta > 0.0) ? AU1 : AU0;
	delta = REMAP (delta, a);
	u = UGRID0 + delta;

	return RESTRICT (u, 0.0, 1.0);
}


double 				/* S-shaped curve in v' */
	fxnull_iFunc_y (double v, fut_calcData_p dataP) 
{
double	delta, a;

	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	v *= KCP_16_TO_8_ENCODING;
#endif
	delta = v - V0;
	a = (delta > 0.0) ? AV1 : AV0;
	delta = REMAP (delta, a);
	v = VGRID0 + delta;

	return RESTRICT (v, 0.0, 1.0);
}


double 				/* linear in L* */
	fxnull_iFunc_z (double L, fut_calcData_p dataP) 
{ 
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	L *= KCP_16_TO_8_ENCODING;
#endif
	return (RESTRICT (L, 0.0, 1.0));
}

/*---------------------------------------------------------------------
 *  fxnull_gFunc_x, fxnull_gFunc_y, fxnull_gFunc_z grid-table functions (identity)
 *---------------------------------------------------------------------
 */

	/* fxnull has an identity grid table so use the existing
		functions lin16_gFunc_x, lin16_gFunc_y, lin16_gFunc_z
		to make the identity grid table.
	*/
	 
/*---------------------------------------------------------------------
 *  fxnull_oFunc -- output-table functions (inverse of input mappings)
 *---------------------------------------------------------------------
 */
double 
	fxnull_oFunc_x (double q, fut_calcData_p dataP) 
{
double 	s;
double	delta, a;

	if (dataP) {}

	s = q;

	delta = s - UGRID0;						/* f(u) -> u */
	a = (delta > 0.0) ? AU1 : AU0;
	delta = UNMAP (delta, a);
	s = U0 + delta;
#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}


double 
	fxnull_oFunc_y (double q, fut_calcData_p dataP) 
{
double 	s;
double	delta, a;

	if (dataP) {}

	s = q;

	delta = s - VGRID0;						/* f(v) -> v */
	a = (delta > 0.0) ? AV1 : AV0;
	delta = UNMAP (delta, a);
	s = V0 + delta;
#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}


double 
	fxnull_oFunc_z (double q, fut_calcData_p dataP) 
{
double 	s;

	if (dataP) {}

	s = q;

#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}
