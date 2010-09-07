/*
 * @(#)lab2uvl.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	lab2uvl.c
 
	Contains:	This module contains functions to create Lab->uvL FuTs.

	From Poe via GBP, 2 Aug 00

	COPYRIGHT (c) 2000-2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcptmgr.h"
#include "makefuts.h"

#define	XWHITE	0.96819					/* Prophecy uvL = [87 196 255] */
#define YWHITE	1.00000
#define	ZWHITE	0.82830

#define	NEUTRALGRID	0.53333333333333	/* 8/15; neutral grid point */
#define M_PI    		3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962


/*---------------------------------------------------------------------
 *  Definitions for a* and b* mappings
 *---------------------------------------------------------------------
 */
#define	NEUTRALBYTE	0.50196078431373	/* 128/255; a* or b* = 0.0 */
#define APOWR		3.0 			/* exponent of a* mapping function */
#define ADENM		19.08553692318767 	/* exp(APOWR) - 1 */
#define	AEXP(x)		((exp(APOWR * (x)) - 1.0) / ADENM)
#define ALOG(x)		(log(ADENM * (x) + 1.0) / APOWR)
#define BPOWR		2.0 			/* exponent of b* mapping function */
#define BDENM		6.38905609893065 	/* exp(BPOWR) - 1 */
#define	BEXP(x)		((exp(BPOWR * (x)) - 1.0) / BDENM)
#define BLOG(x)		(log(BDENM * (x) + 1.0) / BPOWR)


/*--------------------------------------------------------------------
 * LabuvLInit -- Initialize angular offsets
 *--------------------------------------------------------------------
 */
void
	LabuvLInit (LabuvLConst_p c)
{
double	denom;

	denom = XWHITE + 15.0 * YWHITE + 3.0 * ZWHITE;
	c->u_angle_neutral = atan2 (4.0 * XWHITE, denom);
	c->v_angle_neutral = atan2 (9.0 * YWHITE, denom);
	c->u_neutral = tan (c->u_angle_neutral);
	c->v_neutral = tan (c->v_angle_neutral);
}


/*---------------------------------------------------------------------
 *  xfun, yfun, zfun -- input mappings:  Convert a* and b* to unsigned; use S-shaped curve
 *---------------------------------------------------------------------
 */

/* L* */
double
	LabuvL_iL (double L, fut_calcData_p dataP)
{
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	L *= KCP_16_TO_8_ENCODING;
#endif
	return (RESTRICT (L, 0.0, 1.0));
}


/* a* */
double
	LabuvL_ia (double y, fut_calcData_p dataP)
{
	double	delta;

	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	y *= KCP_16_TO_8_ENCODING;
#endif
	delta = y - NEUTRALBYTE;
	if (delta < 0.0) {
		y = NEUTRALGRID * AEXP (y / NEUTRALBYTE);
	}
	else {
		y = 1.0 - (1.0 - NEUTRALGRID) * AEXP ((1.0 - y) / (1.0 - NEUTRALBYTE));
	}

	return RESTRICT (y, 0.0, 1.0);
}


/* b* */
double
	LabuvL_ib (double z, fut_calcData_p dataP)
{
	double	delta;

	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	z *= KCP_16_TO_8_ENCODING;
#endif
	delta = z - NEUTRALBYTE;
	if (delta < 0.0) {
		z = NEUTRALGRID * BEXP (z / NEUTRALBYTE);
	}
	else {
		z = 1.0 - (1.0 - NEUTRALGRID) * BEXP ((1.0 - z) / (1.0 - NEUTRALBYTE));
	}

	return RESTRICT (z, 0.0, 1.0);
}


/*---------------------------------------------------------------------
 *  LabuvL_gFun -- grid-table functions:  L*a*b* -> uvL
 *---------------------------------------------------------------------
 */
double
	LabuvL_gFun (double_p	args, fut_calcData_p dataP)
{
double	l, a, b;	/* representing L* in [0, 100], a* and b* in [-128, 127] */
double	delta, gridval;
double	x, y, z;
double	u, v;		/* output quantities */
double	denom;		/* denominator of u', v' */

	l = args[0];
	a = args[1];
	b = args[2];

    /* Linearize L*:  */
	y = Hinverse (l, &(((auxData_p)dataP)->lc));		/* CIE 1931 Y, in [0, 1] */

    /* Compress luminance for Prophecy uvL:  */
	y = (254.0 * y + 1.0) / 255.0;

	l = Hfunc (y, &(((auxData_p)dataP)->lc));			/* H(Y/Y_n) [compressed or clipped] */ 

	if (((auxData_p) dataP)->std.chan == 2) {
		gridval = l;   /* linear encoding of L* */
	}
	else {
	    /* Undo grid mapping of a* and b*:  */
		delta = a - NEUTRALGRID;
		if (delta < 0.0) {
			a = NEUTRALBYTE * ALOG (a / NEUTRALGRID);
		}
		else {
			a = 1.0 - (1.0 - NEUTRALBYTE) * ALOG ((1.0 - a) / (1.0 - NEUTRALGRID));
		}

		delta = b - NEUTRALGRID;
		if (delta < 0.0) {
			b = NEUTRALBYTE * BLOG (b / NEUTRALGRID);
		}
		else {
			b = 1.0 - (1.0 - NEUTRALBYTE) * BLOG ((1.0 - b) / (1.0 - NEUTRALGRID));
		}

	    /* Shift a* and b* to standard domain:  */
		a = 255.0 * a - 128.0;		/* CIE 1976 a* */
		b = 255.0 * b - 128.0;		/* CIE 1976 b* */

	    /* Rescale a* and b*:  */
		a = 0.00232 * a;		/* H(X/X_n) - H(Y/Y_n) */
		b = 0.00580 * b;		/* H(Y/Y_n) - H(Z/Z_n) */

		/* Separate X and Z channels:  */
		x = a + l;			/* H(X/X_n) */
		z = l - b;			/* H(Z/Z_n) */

	    /* Linearize X and Z:  */
		x = Hinverse (x, &(((auxData_p)dataP)->lc));		/* X/X_n */
		z = Hinverse (z, &(((auxData_p)dataP)->lc));		/* Z/Z_n */

		/* Scale XYZ:  */
		x *= XWHITE;			/* X */
		y *= YWHITE;			/* Y */
		z *= ZWHITE;			/* Z */

		denom = x + 15.0 * y + 3.0 * z;

		/* Compute grid-table entry:  */
		if (((auxData_p) dataP)->std.chan == 0) {
			/* Compute arctan u':  */
			u = fabs (denom);		/* to improve interpolant */
			u = atan2 (4.0 * x, u);	/* arctan u', in [-PI, PI] */

			/* Encode over extended range:  */
			gridval = (u + M_PI_2) / M_PI;	/* [-PI/2, +PI/2] -> [0, 1] */
		}
		else {
			/* Compute arctan v':  */
			v = atan2 (9.0 * y, denom);	/* arctan v', in [-PI, PI] */

			/* Encode over extended range:  */
			gridval = v / M_PI;			/* [0, PI] -> [0, 1] */
		}
	}

	return RESTRICT (gridval, 0.0, 1.0);
}


/* u' */
double
	LabuvL_ou (double p, fut_calcData_p dataP)
{
double	uu;

	if (dataP) {}

	/* Requantize u' to interval [0.07, 0.48]:  */
	uu = M_PI * p - M_PI_2;		/* arctan u' */
	uu = RESTRICT (uu, 0.0, M_PI_4);
	uu = tan (uu);			/* CIE 1976 u', in [0, 1] */
	uu = (uu - 0.070) / 0.40996784565916;

	uu = RESTRICT (uu, 0.0, 1.0);
#if defined KCP_MAP_BASE_MAX_REF
	uu *= KCP_8_TO_16_ENCODING;
#endif
	return uu;
}


/* v' */
double
	LabuvL_ov (double p, fut_calcData_p dataP)
{
double	 vv;

	if (dataP) {}

	/* Requantize v' to interval [0.165, 0.585]:  */
	vv = M_PI *  p;			/* arctan v' */
	vv = RESTRICT (vv, 0.0, M_PI_4);
	vv = tan (vv);			/* CIE 1976 v', in [0, 1] */
	vv = (vv - 0.165) / 0.41986827661910;

	vv = RESTRICT (vv, 0.0, 1.0);
#if defined KCP_MAP_BASE_MAX_REF
	vv *= KCP_8_TO_16_ENCODING;
#endif
	return vv;
}


/* L */
double
	LabuvL_oL (double p, fut_calcData_p dataP)
{
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	p *= KCP_8_TO_16_ENCODING;
#endif
	return RESTRICT (p, 0.0, 1.0);
}
