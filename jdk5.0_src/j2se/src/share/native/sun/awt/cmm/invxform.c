/*
 * @(#)invxform.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)invxform.c	1.24 98/11/04

	Contains:	makeInverseXformFromMatrix

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-2000 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include <string.h>
#include "makefuts.h"


/*---------------------------------------------------------------------------
 *  makeInverseXformFromMatrix -- make a fut of given gridsize from given
 *	matrix data for inverse transform (XYZ -> RGB); return status code
 *---------------------------------------------------------------------------
 */
PTErr_t
	makeInverseXformFromMatrix (LPMATRIXDATA	mdata,
								KpUInt32_t		interpMode,
								KpInt32_p		dim,
								fut_p			theFut)
{
PTErr_t			PTErr = KCP_SUCCESS;
ResponseRecord_p	rrp;
KpInt32_t		i;
fut_chan_p		theChan;
fut_gtbl_p		theGtbl;
fut_otbl_p		theOtbl;
mf2_tbldat_p	gtblDat[3], otblDat, prevOtblDat;
KpUInt16_t		prevGamma = 0, thisGamma;
double			fwdgamma, one[3];
double			offset[3] = {1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
KpUInt16_t		*pCurveData = NULL;

	for (i = 0; i < 3; i++) {
		if (!IS_CHAN(theChan = theFut->chan[i])
			|| !IS_GTBL(theGtbl = theChan->gtbl)
			|| ((gtblDat[i] = theGtbl->refTbl) == NULL) 	/* Get grid tables */
			|| !IS_OTBL(theOtbl = theChan->otbl)
			|| ((otblDat = theOtbl->refTbl) == NULL)) {		/* Get output table */
		   return KCP_INCON_PT;
		}
		
		if (theOtbl->refTblEntries != FUT_OUTTBL_ENT) return KCP_INCON_PT;

		 /* Get ResponseRecord:  */
		rrp = mdata->outResponse[i];
		if (NULL == rrp) {
			break;				/* must only have output tables */
		}
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
		if ((rrp->CurveCount > 0) && (rrp->CurveData == (KpUInt16_p)NULL)) {
			PTErr = KCP_INCON_PT;
			goto ErrOut;
		}

		 /* Recompute output table:  */
		switch (rrp->CurveCount) {
		case 0:	/* linear response, with clipping */
			calcOtbl0 (otblDat);
			break;
			
		case 1:	/* power law */
			thisGamma = rrp->CurveData[0];
			if (prevGamma == thisGamma) {	/* same gamma, just copy table */
				memcpy (otblDat, prevOtblDat, sizeof (*otblDat) * FUT_OUTTBL_ENT);
			}
			else {					
				prevGamma = thisGamma;
				prevOtblDat = otblDat;

				fwdgamma = (double)thisGamma / SCALEDOT8;
				if (fwdgamma <= 0.0) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
				calcOtbl1 (otblDat, fwdgamma);
			}
			break;
			
		default:	/* look-up table of arbitrary length */
			makeInverseMonotonic (rrp->CurveCount, rrp->CurveData);

			if (rrp->CurveCount == theOtbl->refTblEntries) {	/* ready-to-use look-up table */
				memcpy (otblDat, rrp->CurveData, sizeof (*otblDat) * rrp->CurveCount);
			}
			else {
				PTErr = calcOtblN (otblDat, rrp, interpMode);
				if (PTErr != KCP_SUCCESS) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
			}

			break;
		}
	}

	/* Compute inverse matrix (XYZ -> RGB):  */
	one[0] = one[1] = one[2] = 1.0;			/* arbitrary vector */

	 /* replaces matrix with inverse */
	if (solvemat (3, mdata->matrix, one) != 0) {
		PTErr = KCP_INCON_PT;
		goto ErrOut;
	}

	/* Rescale given matrix by factor of 3 for extended range:  */
	for (i = 0; i < 3; i++) {
		KpInt32_t	j;

		for (j = 0; j < 3; j++) {
			mdata->matrix[i][j] /= 3.0;
		}
	}

    /* Replace grid tables:  */
	calcGtbl3 (gtblDat, dim, mdata->matrix, offset);	/* with offset */

ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


/*-------------------------------------------------------------------------------
 *  makeInverseMonotonic -- flatten reversals in data table
 *-------------------------------------------------------------------------------
 */
void
	makeInverseMonotonic (KpUInt32_t count, mf2_tbldat_p table)
{
KpInt32_t		i;
mf2_tbldat_t	val;

     /* Check inputs:  */
	if ((table == (KpUInt16_p)NULL) || (count < 3)) {
		return;
	}

     /* Flatten from high end to low end, depending on polarity:  */
	if (table[0] <= table[count - 1]) {		/* globally non-decreasing */
		val = table[count - 1];
		for (i = count - 2; i >= 0; i--) {	/* from right to left */
			if (table[i] > val) {			/* reversal? */
				table[i] = val;				/* flatten! */
			}
			else {							/* no reversal? */
				val = table[i];				/* update */
			}
		}
	}
	else {									/* globally decreasing */
		val = table[0];
		for (i = 1; i < (KpInt32_t)count; i++)	{		/* from left to right */
			if (table[i] > val)	{			/* reversal? */
				table[i] = val;				/* flatten! */
			}
			else {							/* no reversal? */
				val = table[i];				/* update */
			}
		}
	}
}

