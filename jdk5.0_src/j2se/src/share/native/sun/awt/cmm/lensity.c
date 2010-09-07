/*
 * @(#)lensity.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (c) Copyright 2000 Eastman Kodak - Lowell MA
 *
 *	lensity.hxx
 *
 *	Note:	Taken from Clrlensity.hxx
 *		This class implements the function H(x) used in the CIELAB and
 *		CIELUV systems to model the brightness response of the human
 *		visual system.  It is, essentially, a ClrTransferFunc with
 *		linear extrapolation disabled.  Helper subroutines (Hfunc
 *		and Hinverse) are provided for contexts where construction
 *		of a ClrLensity object is inconvenient.
 *
 */

#include "kcms_sys.h"

#include <math.h>
#include "makefuts.h"


double
	Hfunc (double arg, lensityConst_p lc)
{
double	val;

	if (arg <= lc->x_tan) {
		val = lc->inv_slope * arg;
	}
	else {
		val = (1.0 + lc->offset) * pow (arg, lc->inv_gamma) - lc->offset;
	}

	return val;
}


double
	Hinverse (double arg, lensityConst_p lc)
{
double	val;

	if (arg <= lc->y_tan) {
		val = lc->slope * arg;
	}
	else {
		val = pow ((arg + lc->offset) / (1.0 + lc->offset), lc->gamma);
	}

	return val;
}


/*
 * Initialize lensity constants
 */
void
	lensityInit (lensityConst_p lc)
{
	lc->gamma = 3.0;
	lc->offset = 0.16;

	lc->inv_gamma = 1.0 / lc->gamma;

	lc->y_tan = lc->offset / (lc->gamma - 1.0);
	lc->x_tan = pow (lc->offset * lc->gamma / ((1.0 + lc->offset) * (lc->gamma - 1.0)), lc->gamma);
	lc->slope = lc->x_tan / lc->y_tan;
	lc->inv_slope = 1.0 / lc->slope;
}
