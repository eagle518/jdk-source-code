/*
 * @(#)cmyklin.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	File:		cmyklin.c

	Contains:	Functions used to create the cmyklin and the cmyklin invert
					auxilluary PTs used for chaining

	COPYRIGHT (c) 1991-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "makefuts.h"

#define	LMIN	0.15			/* minimum neutral `lensity' (100% black) */
#define	L0		0.17647			/* LMIN/(1 - LMIN) */
#define YMIN	0.019086		/* H_inverse(LMIN):  minimum reflectance */


/*---------------------------------------------------------------------
 *  cmyklini_iFunc -- input mappings
 *---------------------------------------------------------------------
 */
double 
	cmyklin_iFunc (	double			a,
					fut_calcData_p	dataP)
{
	a = (a + L0) / (1.0 + L0);
	a = Hinverse (a, &(((auxData_p)dataP)->lc));
	a = (a - YMIN) / (1.0 - YMIN);
	return RESTRICT (a, 0.0, 1.0);
}


/*---------------------------------------------------------------------
 *  cmyklini_iFunc (invert) -- input mappings
 *---------------------------------------------------------------------
 */
double 
	cmyklini_iFunc (	double			a,
						fut_calcData_p	dataP)
{
	if (dataP) {}

	a = 1.0 - a; 

	a = (a + L0) / (1.0 + L0);
	a = Hinverse (a, &(((auxData_p)dataP)->lc));
	a = (a - YMIN) / (1.0 - YMIN);
	return RESTRICT (a, 0.0, 1.0);
}

	 
/*---------------------------------------------------------------------
 *  cmyklin_oFunc -- output mapping
 *---------------------------------------------------------------------
 */
double 
	cmyklin_oFunc (	double	s,
					fut_calcData_p	dataP) 
{
	s = (1.0 - YMIN) * s + YMIN;
	s = Hfunc (s, &(((auxData_p)dataP)->lc));
	s = (1.0 + L0) * s - L0;
	s = RESTRICT (s, 0.0, 1.0);

	return s;
}


/*---------------------------------------------------------------------
 *  cmyklini_oFunc -- output mapping (invert)
 *---------------------------------------------------------------------
 */
double 
	cmyklini_oFunc (	double	s,
						fut_calcData_p	dataP) 
{
	s = (1.0 - YMIN) * s + YMIN;
	s = Hfunc (s, &(((auxData_p)dataP)->lc));
	s = (1.0 + L0) * s - L0;
	s = RESTRICT (s, 0.0, 1.0);
	s = 1.0 - s;

	return s;
}

