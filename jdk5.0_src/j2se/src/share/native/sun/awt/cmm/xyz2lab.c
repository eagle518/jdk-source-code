/*
 * @(#)xyz2lab.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)xyz2lab.c	1.21 99/01/06
 
	Contains:	This module contains functions to create an XYZ->lab FuT.

	Created by lsh, November 11, 1993

	COPYRIGHT (c) 1992-2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcptmgr.h"
#include "makefuts.h"

#define KCP_THIS_FUT_CHANS (3)


typedef struct fData_s {
	fut_calcData_t	std;
	KpInt32_t		size[KCP_THIS_FUT_CHANS];
	double			sizeNorm;
	lensityConst_t	lc;
} fData_t, FAR* fData_p;


/*  ifun -- input mappings */
static double ifun (double x, fut_calcData_p dataP)
{
KpInt32_t	chan;

	chan = ((fData_p) dataP)->std.chan;

	switch (chan) {
	case 0:
		x /= (KCP_D50_X * XYZSCALE);	/* X */
		break;

	case 1:
		x /= (KCP_D50_Y * XYZSCALE);	/* Y */
		break;

	case 2:
		x /= (KCP_D50_Z * XYZSCALE);	/* Z */
		break;
	}

	x = Hfunc (x, &(((fData_p)dataP)->lc));
	x *= ((fData_p) dataP)->sizeNorm;

	return RESTRICT (x, 0.0, 1.0);
}


/*  gfun -- grid-table functions:
 *				(H(X/X_n), H(Y/Y_n), H(Z/Z_n)) --> (L*, a*, b*) 
 */
static double gfun (double_p args, fut_calcData_p dataP)
{
double		p, x, y, z;

	x = args[0];
	y = args[1];
	z = args[2];

	y /= ((fData_p) dataP)->sizeNorm;

/* Convert to CIELAB, with L* in [0, 100], a* & b* in [-200, 200]:  */
	switch (((fData_p) dataP)->std.chan) {
    case 0:	/* (L*)/100 */
		p = y;
		break;

   case 1:	/* (2048/4095)[1 + (a*)/200] */
		x /= ((fData_p) dataP)->sizeNorm;
		p = 0.50012210012210 * (1.0 + 2.15517241379310 * (x - y));
		break;

   case 2:	/* (2048/4095)[1 + (b*)/200] */
		z /= ((fData_p) dataP)->sizeNorm;
		p = 0.50012210012210 * (1.0 + 0.86206896551724 * (y - z));
		break;
	}

	p = RESTRICT (p, 0.0, 1.0);	

	return p;
}


/*  outfun -- rescale and clip a* and b* and convert to signed representation
 */
static double ofun (double q, fut_calcData_p dataP)
{
double	p;

	p = q;

	switch (((fData_p) dataP)->std.chan) {
	case 0:
		break;

	case 1:
	case 2:
		p *= 1.99951171875;
		p = 200.0 * (p - 1.0);	/* CIE 1976 a*, b*, in [-200, 200] */
		p = RESTRICT (p, -128.0, 127.0);	/* clip to [-128, 127] */
		p = p + 128.0;		/* -> [0, 255] */
		p /= 255.0;		/* from [0, 255] to [0, 1] */
		break;
	}

	/* L* in [0, 100]; a* & b* in [-128, 127] */
#if defined KCP_MAP_BASE_MAX_REF
	p *= KCP_8_TO_16_ENCODING;
#endif
	p = RESTRICT (p, 0.0, 1.0);	/* superfluous, but whatthehell */
	
	return p;
}


/* get_xyz2lab --	construct an XYZ to Lab FuT
 *						to establish a grid for composition
 */
fut_p
	get_xyz2lab (	KpInt32_t	size)
{
fut_p		futp;
KpInt32_t	iomask;
fData_t		fData;
fut_ifunc_t	ifunArray[KCP_THIS_FUT_CHANS] = {ifun, ifun, ifun};
fut_gfunc_t	gfunArray[KCP_THIS_FUT_CHANS] = {gfun, gfun, gfun};
fut_ofunc_t	ofunArray[KCP_THIS_FUT_CHANS] = {ofun, ofun, ofun};

	#if defined KCP_DIAG_LOG
	kcpDiagLog ("get_xyz2lab\n");
	#endif
	
	iomask = FUT_IN(FUT_XYZ) | FUT_OUT(FUT_XYZ);

	/* all dimensions are the same */
	fData.size[0] = size;
	fData.size[1] = size;
	fData.size[2] = size;
	fData.sizeNorm = (double)(size - 2) / (double)(size - 1);
	lensityInit (&fData.lc);		/* initialize lensity constants */

	/* Compute shared input tables, grid, tables and output tables:  */
	futp = constructfut (iomask, fData.size, &fData.std, ifunArray, gfunArray, ofunArray,
						KCP_XYZ_PCS, KCP_LAB_PCS);

	#if defined KCP_DIAG_LOG
	saveFut (futp, "CP21.fut");
	#endif

	return (futp);
}
