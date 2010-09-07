/*
 * @(#)f4lex.c	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)f4lex.c	1.8 98/10/01

	Contains:	four point Lagrangian interpolater, with extrapolation.

	Written by:	Late Night Mail Order Software Team

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */


#include "kcmsos.h"
#include "makefuts.h"


/*-------------------------------------------------------------------
 *  extrap -- linear extrapolation using endpoint slope of parabola
 *		passing through points 1, 2, & 3
 *-------------------------------------------------------------------
 */
static double extrap (double x, double x1, double  x2, double x3,
							double y1, double y2, double y3)
{
	double	d13, d12, slope;

	if (y2 == y1)		/* if flat at end, . . . */
	    return (y1);		/* . . . assume truncated */

	d12 = x2 - x1;		/* assume x1, x2, x3 all different */
	d13 = x3 - x1;
	slope = ((y2 - y1) * d13 * d13 - (y3 - y1) * d12 * d12)
					/ (d12 * d13 * (d13 - d12));	/* slope at point 1 */
	return (y1 + slope * (x - x1));
}



/*-------------------------------------------------------------------
 *  f4l -- 4-point Lagrangian interpolation, with extrapolation, in
 *		given table of n xy-pairs.  Note:  x-values must be
 *		monotonic.
 *
 *			RFP		September 1987
 *			based on Fortran version by
 *			MSchwartz	January 1984
 *-------------------------------------------------------------------
 */

double f4l (double x, double xtab[], double ytab[], KpInt32_t n, KpInt32_p hint)
{
	KpInt32_t		i, k, l, l1, l2;
	double	p, p2, p3, sum;

     /* Handle special cases:  */
	if (n == 0)	{	/* no table */
#ifdef qDebug
		DebugStr("\p<f4l>:  Empty tables");
#endif
	    return (x);
	}

	if (n == 1)		/* constant function */
	    return (ytab[0]);

	if (n == 2)		/* linear function */
	    return ((ytab[0] * (x - xtab[1]) + ytab[1] * (xtab[0] - x)))
					   / (xtab[0] - xtab[1]);
	/* => n > 2 */

    /* Check for invalid table:  */
	if (xtab[0] == xtab[n - 1]) {
#ifdef qDebug
		DebugStr("\p<f4l>:  Empty domain");
#endif
		return (x);
	}
	/* => xtab[0] != xtab[n - 1] */
	/* Assume strictly monotonic:
		xtab[i] < xtab[j] for all i, j, 0 <= i < j <= (n - 1)  OR
		xtab[i] > xtab[j] for all i, j, 0 <= i < j <= (n - 1)
	*/

     /* Distinguish remaining cases:  */
	if (xtab[0] < xtab[n - 1])	/* ascending order */
	{
		if (x < xtab[0])		/* extrapolate from bottom */
			return (extrap (x, xtab[0], xtab[1], xtab[2],
							   ytab[0], ytab[1], ytab[2]));

		if (x >= xtab[n - 1])	/* extrapolate from top */
			return (extrap (x, xtab[n - 1], xtab[n - 2], xtab[n - 3],
						   ytab[n - 1], ytab[n - 2], ytab[n - 3]));

	   /* => xtab[0] <= x < xtab[n - 1] */

		i = RESTRICT (*hint, 1, n-1);
	    while (x < xtab[i-1])
			i--;			/* find interpolation interval */
	    while (x >= xtab[i])
			i++;			/* find interpolation interval */
		*hint = i;

	}
	else 				/* descending order */
	{
	    if (x <= xtab[n - 1])	/* extrapolate from top */
			return (extrap (x, xtab[n - 1], xtab[n - 2], xtab[n - 3],
					   ytab[n - 1], ytab[n - 2], ytab[n - 3]));

	    if (x > xtab[0])		/* extrapolate from bottom */
			return (extrap (x, xtab[0], xtab[1], xtab[2],
						   ytab[0], ytab[1], ytab[2]));
	   /* => xtab[0] >= x > xtab[n - 1] */

	    i = 1;			/* interpolate */
		while (x <= xtab[i])
			i++;			/* find interpolation interval */
	   /* => xtab[i - 1] >= x > xtab[i] */
	}
	/* => 1 <= i <= (n - 1) */

     /* Compute 4-point Lagrangian interpolant:  */
	l1 = MAX(i - 2, 0);		/* pick end points in [0, n - 1] */
	l2 = MIN(i + 1, n - 1);
	/* => 0 <= l1 < i <= l2 <= (n - 1) */

	sum = 0;			/* accumulate terms */
	for (l = l1; l <= l2; l++)	/* 3 or (usually) 4 points */
	{
	    p = ytab[l];
	    p2 = xtab[l];
	    for (k = l1; k <= l2; k++) {
			if (k != l) {
				p3 = xtab[k];
				p *= (x - p3) / (p2 - p3);
			}
		}
	    sum += p;
	}
	return (sum);
}
