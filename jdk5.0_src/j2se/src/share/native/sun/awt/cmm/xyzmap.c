/*
 * @(#)xyzmap.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	xyzmap.c

	Contains:	Functions used to create the xyzmap auxiliary PT22 used for chaining

 *********************************************************************
 *    COPYRIGHT (c) 1991-2000 Eastman Kodak Company
 *    As an unpublished work pursuant to Title 17 of the United
 *    States Code.  All rights reserved.
 *********************************************************************
*/

#include "makefuts.h"


/*---------------------------------------------------------------------
 *  xyzmap_iFunc -- input mappings
 *---------------------------------------------------------------------
 */
double
	xyzmap_iFunc (double xyz, fut_calcData_p dataP)
{
	switch (dataP->chan) {
	case 0:
		xyz /= (KCP_D50_X * XYZSCALE);	/* X */
		break;

	case 1:
		xyz /= (KCP_D50_Y * XYZSCALE);	/* Y */
		break;

	case 2:
		xyz /= (KCP_D50_Z * XYZSCALE);	/* Z */
		break;
	}

	xyz = Hfunc (xyz, &(((auxData_p)dataP)->lc));
	xyz /= 1.4;

	return RESTRICT (xyz, 0.0, 1.0);
}


/*---------------------------------------------------------------------
 *  xyzmap_oFunc -- output-table functions (inverse of input mappings)
 *---------------------------------------------------------------------
 */
double 
	xyzmap_oFunc (double q, fut_calcData_p dataP) 
{
double 	s;

	s = q;

     /* Recover headroom: */
	s = 1.4 * s;			/* [0, 1] -> [0, 1.4] */

     /* Linearize */
	s = Hinverse (s, &(((auxData_p)dataP)->lc));	/* [0, 1.4] -> [0, 2.4] -> [0, 1.205] */

     /* Interpret according to white point and rescale for ICC:  */
	switch (dataP->chan) {
	case 0:
		s *= (KCP_D50_X * XYZSCALE);	/* X */
		break;

	case 1:
		s *= (KCP_D50_Y * XYZSCALE);	/* Y */
		break;

	case 2:
		s *= (KCP_D50_Z * XYZSCALE);	/* Z */
		break;
	}

	s = RESTRICT (s, 0.0, 1.0);
	return s;
}
