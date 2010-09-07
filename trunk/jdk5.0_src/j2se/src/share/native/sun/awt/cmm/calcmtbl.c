/*
 * @(#)calcmtbl.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	calcmtbl.c

	Contains:	sets up the ouput tables for monochrome	transforms

	Written by:	Color Processor group

	COPYRIGHT (c) 1997-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcms_sys.h"

#include <math.h>
#include "makefuts.h"

/*---------------------------------------------------------------------------
 *  calcOtblLSN -- calculate an output table by doing a device to L(TRC) & L to
 					L*  conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t
	calcOtblLSN (mf2_tbldat_p table, ResponseRecord_p rrp)
{
PTErr_t			PTErr = KCP_SUCCESS;
mf2_tbldat_p	data;
KpInt32_t		count, ix, length;
double			val, x, frac, nument_inv_li, dblMaxVal = MF2_TBL_MAXVAL;
KpBool_t		decreasing;
lensityConst_t	lc;
KpUInt16_t		*pCurveData = NULL;

	/* Check input and initialize:  */
	if (table == NULL) return KCP_BAD_ARG;	/* absent table */
	if (rrp == NULL) return KCP_BAD_ARG;
	
	if (PARA_TYPE_SIG == rrp->TagSig)
	{
		pCurveData = (KpUInt16_p) allocBufferPtr (MFV_CURVE_TBL_ENT);	/* get memory for curve data */
		if (NULL == pCurveData) {
			return KCP_NO_MEMORY;
		}
		makeCurveFromPara (rrp->ParaFunction, rrp->ParaParams, pCurveData, MFV_CURVE_TBL_ENT);
		rrp->CurveCount = MFV_CURVE_TBL_ENT;
		rrp->CurveData = pCurveData;
	}
	length = (KpInt32_t) rrp->CurveCount;
	data = rrp->CurveData;

	if ((length == 0) ||							/* bad size */
		(data == NULL) ||							/* absent table */
		(data[length - 1] == data[0])) {			/* empty domain */
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}

	if (data[0] > data[length-1]) {
		decreasing = KPTRUE;
	} else {
		decreasing = KPFALSE;
	}

	lensityInit (&lc);		/* initialize lensity constants */
	nument_inv_li = (double)(length - 1) / (double)(FUT_OUTTBL_ENT - 1);
	
     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {

	 	/* Compute linearized value by interpolating in transfer table:  */
		x = (double)count * nument_inv_li;		/* in [0, length - 1] */
		ix = (KpInt32_t)x;						/* integer part */
		if (ix >= (length - 1)) {				/* off end of data table */
			val = (double)data[length - 1];		/* take last value */
		}
		else {									/* within data table, interpolate */
			frac = x - (double)ix;				/* fractional part */
			val = (double)data[ix] + frac * ((double)data[ix + 1] - (double)data[ix]);	/* in [0, 1] */
		}

		/* scale and limit slope */
		val /= dblMaxVal;
		val = RESTRICT(val, 0.0, 1.0);
	
		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MAX (val, (1-x)/SLOPE_LIMIT);			/* limit the decreasing slope */
			} else {
				val = MIN (val, x*SLOPE_LIMIT);				/* limit the increasing slope */
			}
		}

		val = Hfunc (val, &lc);		/* calculate L to L* */

		*(table++) = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */
	}
	
	PTErr = KCP_SUCCESS;
	
ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


/*---------------------------------------------------------------------------
 *  calcOtblLS1 -- calculate an output table gamma value by doing a power law & L to
 					L*  conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t
	calcOtblLS1 (mf2_tbldat_p table, double gamma)
{
KpInt32_t	count;
double		val, x, nument_inv, dblMaxVal = MF2_TBL_MAXVAL;
lensityConst_t	lc;

     /* Check input parameters and initialize:  */
	if (table == (mf2_tbldat_p)NULL) return KCP_BAD_ARG;				/* just don't crash */
	if (gamma == 0.0) return KCP_BAD_ARG;

	lensityInit (&lc);		/* initialize lensity constants */
	nument_inv = 1.0 / (double)(FUT_OUTTBL_ENT - 1);

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {
		x = (double)count * nument_inv;		/* in [0, 1] */

		/* Compute linearized value by power law:  */
		val = pow (x, gamma);				/* in [0, 1] */
		val = MIN (val, x*SLOPE_LIMIT);		/* limit the slope */

		/* calculate L to L* */
		val = RESTRICT(val, 0.0, 1.0);
		val = Hfunc (val, &lc);
	
		*(table++) = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */
	}

	return KCP_SUCCESS;
}


/*---------------------------------------------------------------------------
 *  calcOtblLN -- calculate an output table by doing a L* to L & 
 					L to device(inverted TRC) conversion.
 *---------------------------------------------------------------------------
 */

PTErr_t
	calcOtblLN (mf2_tbldat_p table, ResponseRecord_p rrp)
{
PTErr_t			PTErr = KCP_SUCCESS;
mf2_tbldat_p	data;
KpInt32_t		count, length;
double			val, p, nument_inv, length_inv, dblMaxVal = MF2_TBL_MAXVAL;
KpBool_t		decreasing;
lensityConst_t	lc;
KpUInt16_t		*pCurveData = NULL;

	/* Check input and initialize:  */
	if (table == NULL) return KCP_BAD_ARG;	/* absent table */
	if (rrp == NULL) return KCP_BAD_ARG;
	
	if (PARA_TYPE_SIG == rrp->TagSig)
	{
		pCurveData = (KpUInt16_p) allocBufferPtr (MFV_CURVE_TBL_ENT);	/* get memory for curve data */
		if (NULL == pCurveData) {
			return KCP_NO_MEMORY;
		}
		makeCurveFromPara (rrp->ParaFunction, rrp->ParaParams, pCurveData, MFV_CURVE_TBL_ENT);
		rrp->CurveCount = MFV_CURVE_TBL_ENT;
		rrp->CurveData = pCurveData;
	}
	length = (KpInt32_t) rrp->CurveCount;
	data = rrp->CurveData;

	if ((length == 0) ||							/* bad size */
		(data == NULL) ||							/* absent table */
		(data[length - 1] == data[0])) {			/* empty domain */
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}

	if (data[0] > data[length-1]) {
		decreasing = KPTRUE;
	} else {
		decreasing = KPFALSE;
	}

	lensityInit (&lc);		/* initialize lensity constants */
	nument_inv = 1.0 / (double)(FUT_OUTTBL_ENT - 1);
	length_inv = 1.0 / (double)(length - 1);
	
     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {
		
		/* scale and calcuate L* to L */
		p = (double)count * nument_inv;
		p = Hinverse (p, &lc);
		p = RESTRICT (p, 0.0, 1.0);

		/* Find value relative to data table:  */
		val = calcInvertTRC (p * dblMaxVal, data, length);

		/* rescale and clip to [0, 1] */
		val *= length_inv;

		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MAX (val, (1-p)/SLOPE_LIMIT);			/* limit the decreasing slope */
			} else {
				val = MIN (val, p*SLOPE_LIMIT);				/* limit the increasing slope */
			}
		}

		*(table++) = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */
	}
	
	PTErr = KCP_SUCCESS;
	
ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


PTErr_t
	calcOtblL1 (mf2_tbldat_p table, double gamma)
{
KpInt32_t		count;
double			val, invgamma, x, p, nument_inv, dblMaxVal = MF2_TBL_MAXVAL;
lensityConst_t	lc;

     /* Check input parameters and initialize:  */
	if (table == (mf2_tbldat_p)NULL) return KCP_BAD_ARG;		/* just don't crash */
	if (gamma == 0.0) return KCP_BAD_ARG;

	/* invert the gamma */
	lensityInit (&lc);		/* initialize lensity constants */
	invgamma = 1.0 / gamma;
	nument_inv = 1.0 / (double)(FUT_OUTTBL_ENT - 1);

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < FUT_OUTTBL_ENT; count++) {
		p = (double)count * nument_inv;

		x = Hinverse (p, &lc);		/* calculate L* to L, in [0, 1] */

		/* Compute linearized value by power law */
		val = pow (x, invgamma);					/* in [0, 1] */
		val = MIN (val, x*SLOPE_LIMIT);				/* limit the slope */
	
		*(table++) = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */
	}

	return KCP_SUCCESS;
}


/* output table function */
double otblFunc
	(double q, fut_calcData_p dataP)
{
	if (q) {}
	if (dataP) {}

	return 0.50000762951;	/* = (double) (0x8000) / (double) (MF2_TBL_MAXVAL) */
}
