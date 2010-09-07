/*
 * @(#)loguvl.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	loguvl.c

	Contains:	Functions used to create the loguvl auxilluary PT used for chaining

	COPYRIGHT (c) 1991-2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "makefuts.h"

/*----------------------------------------------------------------------
 *  Definitions for RGB input mapping
 *----------------------------------------------------------------------
 */

#define	LOGMAP(x)	( ((x) > FLARE_TANPT) ? 1.0 + log10 (x) / FLARE_DMAX : FLARE_SLOPE * (x) )
#define	UNMAP(x)	( ((x) > FLARE_MAPPT) ? ANTILOG (FLARE_DMAX * ((x) - 1.0)) : (x) / FLARE_SLOPE )

/*---------------------------------------------------------------------
 *  Definitions for grid-table chromaticity encoding
 *---------------------------------------------------------------------
 */
#define U_n			0.20			/* arbitrary neutral:  u = u' = 0.20 */
#define V_n			0.48			/*	v = 0.32, v' = 0.48 */
#define UVMIN		0.00			/* represent u' and v' in [0, 0.6] */
#define UVMAX		0.60
#define UFACTOR		2.00			/* (u'_max - u'_n)/(u'_n - u'_min) = (UVMAX - U_n)/(U_n - UVMIN) */
#define VFACTOR		0.25			/* (v'_max - v'_n)/(v'_n - v'_min) */


/*---------------------------------------------------------------------
 *  xfun, yfun, zfun -- quasi-logarithmic input mappings
 *---------------------------------------------------------------------
 */
double
	loguvl_iFunc_x (double u, fut_calcData_p dataP)			/* Prophecy encoding of u' */
{
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	u *= KCP_16_TO_8_ENCODING;
#endif
	u = 0.070 + 0.41 * u;			/* CIE 1976 u' */
	u = RESTRICT (u, 0.0, UVMAX - 0.00001);
	u = UFACTOR * u / (UVMAX - u);
	u = 0.5 * (1.0 + log10 (u) / FLARE_DMAX);
	u = (u - 0.379879870981772116) / 0.307752563817030422;

	return RESTRICT (u, 0.0, 1.0);
}

double
	loguvl_iFunc_y (double v, fut_calcData_p dataP)			/* Prophecy encoding of v' */
{
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	v *= KCP_16_TO_8_ENCODING;
#endif
	v = 0.165 + 0.42 * v;			/* CIE 1976 v' */
	v = RESTRICT (v, 0.0, UVMAX - 0.00001);
	v = VFACTOR * v / (UVMAX - v);
	v = 0.5 * (1.0 + log10 (v) / FLARE_DMAX);
	v = (v - 0.287440635235059139) / 0.418042037304444891;

	return RESTRICT (v, 0.0, 1.0);
}

double
	loguvl_iFunc_z (double L, fut_calcData_p dataP)		/* Prophecy encoding of L* */
{
	if (dataP) {}

#if defined KCP_MAP_BASE_MAX_REF
	L *= KCP_16_TO_8_ENCODING;
#endif
	L = Hinverse (L, &(((auxData_p)dataP)->lc));	/* CIE 1931 Y */
	L =  LOGMAP (L);

	return (RESTRICT (L, 0.0, 1.0));
}

/*---------------------------------------------------------------------
 *  trifun -- grid-table functions (identity)
 *---------------------------------------------------------------------
 */

	/* fxnull has an identity grid table so use the existing
		functions lin16_gFunc_x, lin16_gFunc_y, lin16_gFunc_z
		to make the identity grid table.
	*/
	 
/*---------------------------------------------------------------------
 *  outfun -- output mapping (same as in moninputpt; no tonal transfer)
 *---------------------------------------------------------------------
 */
double
	loguvl_oFunc_x (double q, fut_calcData_p dataP)
{
double s;

	if (dataP) {}

	s = q;

	s = 0.307752563817030422 * s + 0.379879870981772116;
	s = ANTILOG (2.0 * FLARE_DMAX * (s - 0.5));
	s = UVMAX * s / (s + UFACTOR);		/* CIE 1976 u' */
	s = (s - 0.070) / 0.41;
#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}

double
	loguvl_oFunc_y (double q, fut_calcData_p dataP)
{
double s;

	if (dataP) {}

	s = q;

	s = 0.418042037304444891 * s + 0.287440635235059139;
	s = ANTILOG (2.0 * FLARE_DMAX * (s - 0.5));
	s = UVMAX * s / (s + VFACTOR);		/* CIE 1976 v' */
	s = (s - 0.165) / 0.42;
#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}

double
	loguvl_oFunc_z (double q, fut_calcData_p dataP)
{
double s;

	if (dataP) {}

	s = q;

	s = UNMAP (s);				/* CIE 1931 Y */
	s = Hfunc (s, &(((auxData_p)dataP)->lc));	/* CIE 1976 (L*)/100 */
#if defined KCP_MAP_BASE_MAX_REF
	s *= KCP_8_TO_16_ENCODING;
#endif
	return (RESTRICT (s, 0.0, 1.0));
}
