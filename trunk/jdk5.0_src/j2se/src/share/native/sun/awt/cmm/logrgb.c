/*
 * @(#)logrgb.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	logrgb.c

	Contains:	Functions used to create the logrgb auxilluary PT used for chaining

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "makefuts.h"

/*----------------------------------------------------------------------
 *  Definitions for RGB input mapping
 *----------------------------------------------------------------------
 */
#define	GAMMA			2.222222222
#define INVGAMMA		0.45				/* = 1.0 / GAMMA */
#define	X0				0.018
#define GSLOPE			4.506813191
#define	Y0				0.081122637			/* = GSLOPE * X0 */
#define	OFFSET			0.09914989
#define	SCALE			1.09914989			/* = 1.0 + OFFSET */

#define CCIR(x)		( ((x)>X0) ? SCALE * POW ((x), INVGAMMA) - OFFSET \
								 : GSLOPE * (x) )
#define CCIRLIN(y)	( ((y)>Y0) ? POW (((y) + OFFSET) / SCALE, GAMMA) \
							     : (y) / GSLOPE )
#define	FLARE(x)	( ((x)<FLARE_TANPT) ? pow (FLARE_YMIN, 1.0 - FLARE_SLOPE * (x)) \
										  : (x) )
#define DEFLARE(x)	( ((x)<FLARE_TANPT)	? (1.0 + log10 (x) / FLARE_DMAX) / FLARE_SLOPE \
										  : (x) )

/*---------------------------------------------------------------------
 *  logrgb_iFunc -- input mappings
 *	 The same input function is used for all three input tables
 *---------------------------------------------------------------------
 */
double
	logrgb_iFunc (double x, fut_calcData_p dataP)
{ 
	if (dataP) {}

	x = CCIRLIN (x);
	x = FLARE (x);
	x = 1.0 + log10 (x) / FLARE_DMAX;
	return RESTRICT (x, 0.0, 1.0);
}

/*---------------------------------------------------------------------
 *  logrgb_gFunc_x, logrgb_gFunc_y, logrgb_gFunc_z grid-table functions (identity)
 *---------------------------------------------------------------------
 */

	/* fxnull has an identity grid table so use the existing
		functions lin16_gFunc_x, lin16_gFunc_y, lin16_gFunc_z
		to make the identity grid table.
	*/
	 
/*---------------------------------------------------------------------
 *  logrgb_oFunc -- output-table functions (inverse of input mappings)
 *---------------------------------------------------------------------
 */
double 
	logrgb_oFunc (double q, fut_calcData_p dataP) 
{
double s; 

	if (dataP) {}

	s = q;

	s = ANTILOG (FLARE_DMAX * (s - 1.0));
	s = DEFLARE (s);
	s = CCIR (s); 

	s = RESTRICT (s, 0.0, 1.0);
	return s;
}

