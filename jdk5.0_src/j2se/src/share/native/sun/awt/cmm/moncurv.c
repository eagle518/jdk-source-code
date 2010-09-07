/*
 * @(#)moncurv.c	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)moncurv.c	1.4 99/01/19
 
	Contains:	create an identity fut with ITU-R BT.709 transfer function on input.

	Created by gbp, 30 Sep 98

	COPYRIGHT (c) 1998-1999 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.

*/

/* from matlab:

forward:
xb = k/(gam-1);
fs = ((gam-1)/k)*(k*gam/((gam-1)*(1+k)))^gam;
ind = x <= xb;
out(ind) = fs * x(ind);
out(~ind) = ((x(~ind) + k)/(1+k)).^gam;

reverse:
yb = (k*gam/((gam-1)*(1+k)))^gam;
rs = (((gam-1)/k)^(gam-1))*((1+k)/gam)^gam;
ind = x <= yb;
out(ind) = rs * x(ind);
out(~ind) = (1+k)*x(~ind).^(1/gam) - k;

*/

#include <stdio.h>
#include "kcptmgr.h"
#include "makefuts.h"

typedef struct fData_s {
	fut_calcData_t	std;
	double			fBreak;
	double			fSlope;
	double			rBreak;
	double			rSlope;
	double			k;
	double			kPlusOne;
	double			gamma;
	double			invGamma;
} fData_t, FAR* fData_p;


/* forward gamma function */
static double
	gammaFunc (double x, fut_calcData_p dataP)
{
double	out, fBreak, fSlope, k, kPlusOne, gamma;

	fBreak = ((fData_p) dataP)->fBreak;

	if (x < fBreak) {
		fSlope = ((fData_p) dataP)->fSlope;

		out = x * fSlope;
	}
	else {
		k = ((fData_p) dataP)->k;
		kPlusOne = ((fData_p) dataP)->kPlusOne;
		gamma = ((fData_p) dataP)->gamma;

		out = (x + k) / kPlusOne;
		out = (double) pow (out, gamma);
	}

	return out;
}


/* inverse gamma function */
static double
	invGammaFunc (double gData, fut_calcData_p dataP)
{
double	x, out, rBreak, rSlope, k, kPlusOne, invGamma;

	x = gData;

	rBreak = ((fData_p) dataP)->rBreak;

	if (x < rBreak) {
		rSlope = ((fData_p) dataP)->rSlope;

		out = x * rSlope;
	}
	else {
		k = ((fData_p) dataP)->k;
		kPlusOne = ((fData_p) dataP)->kPlusOne;
		invGamma = ((fData_p) dataP)->invGamma;

		out = (double) pow (x, invGamma);
		out = (kPlusOne * out) - k;
	}

	out = RESTRICT (out, 0.0, 1.0);
	return out;
}


/* this actually builds the fut */

#define KCP_THIS_FUT_CHANS (3)

fut_p
	get_idenMonCurv_fut (	KpInt32_t	size,
							double		invGamma,
							double		offset)
{
fut_p		futp;
KpInt32_t	iomask, sizeArray[KCP_THIS_FUT_CHANS];
fut_ifunc_t	ifunArray[KCP_THIS_FUT_CHANS] = {gammaFunc, gammaFunc, gammaFunc};
fut_ofunc_t	ofunArray[KCP_THIS_FUT_CHANS] = {invGammaFunc, invGammaFunc, invGammaFunc};
fData_t		fData;
double		k, kPlusOne, gamma, tmpF0, tmpF1, tmpF2, tmpF3, tmpF4;

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "get_idenMonCurv_fut\n offset %f, invGamma %f\n", offset, invGamma);
	kcpDiagLog (string); }
	#endif

	/* set up Rec 709 constants */
	fData.k = k = offset;
	fData.invGamma = invGamma;
	fData.kPlusOne = kPlusOne = 1.0 + k;
	fData.gamma = gamma = 1 / invGamma;

	tmpF4 = gamma - 1.0;
	tmpF0 = tmpF4 / k;
	fData.fBreak = 1.0 / tmpF0;

	tmpF1 = (double) pow ((k * gamma) / (tmpF4 * kPlusOne), gamma);
	fData.fSlope = tmpF0 * tmpF1;
	
	fData.rBreak = tmpF1;

	tmpF2 = (double) pow (tmpF0, tmpF4);
	tmpF3 = (double) pow (kPlusOne / gamma, gamma);
	fData.rSlope = tmpF2 * tmpF3;

	iomask = FUT_IN(FUT_XYZ) | FUT_OUT(FUT_XYZ);

	/* assume all dimensions are the same */
	sizeArray[0] = size;
	sizeArray[1] = size;
	sizeArray[2] = size;

	futp = constructfut (iomask, sizeArray, &fData.std, ifunArray, NULL, ofunArray,
						KCP_FIXED_RANGE, KCP_FIXED_RANGE);

	#if defined KCP_DIAG_LOG
	saveFut (futp, "rec709.fut");
	#endif
				
	return (futp);
}
