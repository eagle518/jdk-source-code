/*
 * @(#)calcotbl.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	calcotbl.c

	Contains:	calcOtbl0, calcOtbl1, calcOtblN

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

/*
 *	General definitions
 */

#include <math.h>
#include "makefuts.h"

static KpInt32_t initOTable (mf2_tbldat_p* tableP, double first, double last);

/*---------------------------------------------------------------------------
 *  calcOtbl0, calcOtbl1, calcOtblN -- calculate an output table
 *		to clip extended range, gamma-correct RGB outputs according to 
 *		specified algorithm (linear, power law, or transfer table),
 *		and requantize.
 *---------------------------------------------------------------------------
 */

void
	calcOtbl0 (mf2_tbldat_p table)
{
KpInt32_t		count;
double			p, nument_inv, dblMaxVal = (double) MF2_TBL_MAXVAL;
mf2_tbldat_t	odata;

	if (table == (mf2_tbldat_p)NULL) return;	/* just don't crash! */

	count = initOTable (&table, 0.0, 1.0);

	nument_inv = 1.0 / (double) (FUT_OUTTBL_ENT -1);

	/* middle third is non-clipped data */
	for (; count < FUT_OUTTBL_ENT -1; count += 3) {
	    /* Treat otbl index as output from a gtbl scaled to FUT_OUTTBL_ENT -1:  */
		p = (double)count * nument_inv;		/* in [0, 1] */

		odata = QUANT_MF2 (p, dblMaxVal);	/* Rescale to max */

		*(table++) = odata;
	}
}


void
	calcOtbl1 (mf2_tbldat_p table, double fwdgamma)
{
KpInt32_t		count;
double			p, val, invgamma, nument_inv, dblMaxVal;
mf2_tbldat_t	odata;

    /* Check input parameters:  */
	if (table == (mf2_tbldat_p)NULL) return;	/* just don't crash! */
	if (fwdgamma == 0.0) return;

	if (fwdgamma == 1.0) {	/* trivial */
	   calcOtbl0 (table);
	   return;
	}

	count = initOTable (&table, 0.0, 1.0);

	invgamma = 1.0 / fwdgamma;
	nument_inv = 1.0 / (double) (FUT_OUTTBL_ENT -1);
	dblMaxVal = (double) MF2_TBL_MAXVAL;

	/* middle third */
	for (; count < FUT_OUTTBL_ENT -1; count += 3) {

	     /* Treat otbl index as output from a gtbl scaled to 4095:  */
		p = (double)count * nument_inv;	/* in [0, 1] */

	     /* compute correction from inverse power law:  */
		val = (double)pow (p, invgamma);

		val = MIN (val, p * SLOPE_LIMIT);	/* limit the slope */

		odata = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */

		*table = odata;						/* in [0, norm - 1] */

		table++;
	}
}


PTErr_t
	calcOtblN (mf2_tbldat_p table, ResponseRecord_p rrp, KpUInt32_t interpMode)
{
PTErr_t			PTErr = KCP_SUCCESS;
KpUInt32_t		length;
KpUInt16_p		data;
KpInt32_t		count, hint = 1;
double			p, val, nument_inv, dblMaxVal, length_inv;
xfer_t			Xfer;
mf2_tbldat_t	odata;
KpBool_t		decreasing;
double			first, last;
KpUInt16_t		*pCurveData = NULL;

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
	length = rrp->CurveCount;
	data = rrp->CurveData;

	if (length == 0) {
		PTErr = KCP_BAD_ARG;					/* bad size */
		goto ErrOut;
	}
	if (data == NULL) {
		PTErr = KCP_BAD_ARG;					/* absent table */
		goto ErrOut;
	}

	if (data[length - 1] == data[0]) {
		PTErr = KCP_BAD_ARG;					/* empty domain */
		goto ErrOut;
	}

	if (data[0] > data[length-1]) {
		decreasing = KPTRUE;
	} else {
		decreasing = KPFALSE;
	}

	/* ==> (length > 1) && (data[length - 1] != data[0]) */
	/* assume monotonically nondecreasing or nonincreasing */

	nument_inv = 1.0 / (double)(FUT_OUTTBL_ENT -1);
	length_inv = 1.0 / (double)(length - 1);
	dblMaxVal = (double) MF2_TBL_MAXVAL;

	switch (interpMode) {
	case KCP_TRC_LINEAR_INTERP:

		val = calcInvertTRC (0.0, data, length);
		first = val * length_inv;
		
		val = calcInvertTRC (dblMaxVal, data, length);
		last = val * length_inv;
		
		count = initOTable (&table, first, last);

		/* middle third */
		for (; count < FUT_OUTTBL_ENT -1; count += 3) {
			p = (double)count * nument_inv;		/* in [0, 1] */

			/* Find value relative to data table:  */
			val = calcInvertTRC (p * dblMaxVal, data, length);
			val *= length_inv;	/* Rescale and clip to [0, 1] */

			if (length < SLOPE_COUNT) {
				if (decreasing == KPTRUE) {
					val = MAX (val, (1-p)/SLOPE_LIMIT);		/* limit the decreasing slope */
				}
				else {
					val = MIN (val, p*SLOPE_LIMIT);			/* limit the increasing slope */
				}
			}

			odata = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */

			*table = odata;						/* in [0, norm - 1] */
	
			table++;
		}

		break;
		
	case KCP_TRC_LAGRANGE4_INTERP:
	
/* !!!!  LAGRANGIAN DOES NOT WORK IF THE SOURCE DATA IS NON-MONOTONIC OR HAS FLAT REGIONS !!!! */

		PTErr = init_xfer (&Xfer, rrp);
		if (PTErr != KCP_SUCCESS) {
			PTErr = KCP_BAD_ARG;
			goto ErrOut;
		}
		
		PTErr = set_xfer (&Xfer, 1, 0);
		if (PTErr != KCP_SUCCESS) {
			PTErr = KCP_BAD_ARG;
			goto ErrOut;
		}

		first = xfer (&Xfer, 0.0, &hint);
		last = xfer (&Xfer, 1.0, &hint);
		
		count = initOTable (&table, first, last);
	
		/* middle third */
		for (; count < FUT_OUTTBL_ENT -1; count += 3) {

		     /* Treat otbl index as output from a gtbl scaled to 4095:  */
			p = (double)count * nument_inv;		/* in [0, 1] */
	
		     /* Clip and compute correction from inverse power law:  */
			val = xfer (&Xfer, p, &hint);
	
			if (length < SLOPE_COUNT) {
				if (decreasing == KPTRUE) {
					val = MAX (val, (1-p)/SLOPE_LIMIT);		/* limit the decreasing slope */
				}
				else {
					val = MIN (val, p*SLOPE_LIMIT);			/* limit the increasing slope */
				}
			}

			odata = QUANT_MF2 (val, dblMaxVal);	/* Rescale to max */

			*table = odata;						/* in [0, norm - 1] */
	
			table++;
		}
	
		break;
		
	default:
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}
	
ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


static KpInt32_t
	initOTable (mf2_tbldat_p*	tableP,
				double			firstD,
				double			lastD)
{
KpInt32_t		count, retCount;
mf2_tbldat_p	table = *tableP;
mf2_tbldat_t	first, last;
double			dblMaxVal = (double) MF2_TBL_MAXVAL;

	/* first third */
	first = QUANT_MF2 (firstD, dblMaxVal);
	for (count = -(FUT_OUTTBL_ENT -1); count <= 0; count += 3) {
		*(table++) = first;
	}

	retCount = count;	/* where the real thing starts */
	*tableP = table;

	/* skip middle third */
	for (; count < FUT_OUTTBL_ENT -1; count += 3) {
		table++;
	}

	/* last third */
	last = QUANT_MF2 (lastD, dblMaxVal);
	for (; count < 2 * FUT_OUTTBL_ENT; count += 3) {
		*(table++) = last;
	}

	return retCount;
}


double
	calcInvertTRC (double p, mf2_tbldat_p data, KpUInt32_t length)
{
KpInt32_t	i, j;
double		val;

	if (data[length - 1] > data[0])		/* monotonic nondecreasing */
	{
		if (p <= (double)data[0])			/* at bottom or below table */
		{
			p = (double)data[0];			/* clip to bottom */
			i = 0;
			while ((double)data[i + 1] <= p)	/* find last bottom entry */
				i++;
			/* ==> data[i] == p < data[i + 1] */
			val = (double)i;
		}
		else if (p >= (double)data[length - 1])	/* at top or above table */
		{
			p = (double)data[length - 1];		/* clip to top */
			i = length - 1;
			while ((double)data[i - 1] >= p)	/* find first top entry */
				i--;
			/* ==> data[i] == p > data[i - 1] */
			val = (double)i;
		}
		else	/* data[0] < p < data[length - 1] */	/* within table */
		{
			i = 1;
			while (p > (double)data[i])		/* find upper bound */
				i++;
			/* ==> data[i - 1] < p <= data[i] */
	
			if (p < (double)data[i])		/* data[i - 1] < p < data[i] */
			{
				val = (double)(i - 1)		/* interpolate in [i - 1, i] */
					+ (p - (double)data[i - 1])
					/ ((double)data[i] - (double)data[i - 1]);
			}
			else					/* p == data[i] */
			{
				j = i;				/* find end of flat spot */
				while (p >= (double)data[j + 1])
					j++;
				/* ==> data[i - 1] < data[i] == p == data[j] < data[j + 1] */
				val = 0.5 * (double)(i + j);		/* pick midpoint of [i, j] */
			}
		}
	}
	else if (data[0] > data[length - 1]) {	/* monotonic nonincreasing */
		if (p <= (double)data[length - 1])		/* at bottom or below table */
		{
			p = (double)data[length - 1];		/* clip to bottom */
			i = length - 1;
			while ((double)data[i - 1] <= p)	/* find first bottom entry */
				i--;
			/* ==> data[i] == p < data[i - 1] */
			val = (double)i;
		}
		else if (p >= (double)data[0])		/* at top or above table */
		{
			p = (double)data[0];			/* clip to top */
			i = 0;
			while ((double)data[i + 1] >= p)	/* find last top entry */
				i++;
			/* ==> data[i] == p > data[i + 1] */
			val = (double)i;
		}
		else	/* data[0] > p > data[length - 1] */	/* within table */
		{
			i = 1;
			while (p < (double)data[i])		/* find upper bound */
				i++;
			/* ==> data[i - 1] > p >= data[i] */
	
			if (p > (double)data[i])		/* data[i - 1] > p > data[i] */
			{
				val = (double)(i - 1)		/* interpolate in [i - 1, i] */
					+ (p - (double)data[i - 1])
					/ ((double)data[i] - (double)data[i - 1]);
			}
			else					/* p == data[i] */
			{
				j = i;				/* find end of flat spot */
				while (p <= (double)data[j + 1])
					j++;
				/* ==> data[i - 1] > data[i] == p == data[j] > data[j + 1] */
				val = 0.5 * (double)(i + j);		/* pick midpoint of [i, j] */
			}
		}	
	}
	else { /* data[0] == data[length - 1] */
		/* return midpoint */
		val = ((double)length)/2 + 0.5F;
	}

	return (val);
}
