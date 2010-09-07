/*
 * @(#)fwdxform.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)fwdxform.c	1.26 99/01/08

	Contains:	makeForwardXformFromMatrix

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include <string.h>
#include "makefuts.h"


/*---------------------------------------------------------------------------
 *  makeForwardXformFromMatrix -- make a fut of given gridsize from given matrix data
 *		for forward transform (RGB -> XYZ); return status code
 *---------------------------------------------------------------------------
 */
PTErr_t
	makeForwardXformFromMatrix (LPMATRIXDATA	mdata,
								KpUInt32_t		interpMode,
								KpInt32_p		dim,
								fut_p			theFut)
{
PTErr_t			PTErr = KCP_SUCCESS;
ResponseRecord_p	rrp;
KpInt32_t		i;
fut_itbl_p		theItbl;
fut_chan_p		theChan;
fut_gtbl_p		theGtbl;
mf2_tbldat_p	itblDat, prevItblDat, gtblDat[3];
KpUInt16_t		prevGamma = 0, thisGamma;
double			fwdgamma;
double			offset[3] = {0.0, 0.0, 0.0};
KpUInt16_t		*pCurveData = NULL;

	for (i = 0; i < 3; i++) {
		if (!IS_ITBL(theItbl = theFut->itbl[i])
			|| ((itblDat = theItbl->refTbl) == NULL)) {	/* Get input table */
		   return KCP_INCON_PT;
		}

		 /* Get ResponseRecord:  */
		rrp = mdata->inResponse[i];
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

		 /* Recompute input table:  */
		switch (rrp->CurveCount) {
		case 0:	/* linear response */
			/* no-op:  leave ramps alone */
			break;

		case 1:	/* power law */
			thisGamma = rrp->CurveData[0];
			if (prevGamma == thisGamma) {	/* same gamma, just copy table */
				memcpy (itblDat, prevItblDat, sizeof (*itblDat) * FUT_INPTBL_ENT);
			}
			else {					
				prevGamma = thisGamma;
				prevItblDat = itblDat;

				fwdgamma = (double)thisGamma / SCALEDOT8;
				if (fwdgamma <= 0.0) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
				PTErr = PTCreateTRC (itblDat, fwdgamma);
				if (PTErr != KCP_SUCCESS) {
					goto ErrOut;
				}
			}
			break;

		default:	/* transfer table of arbitrary length */
			makeMonotonic (rrp->CurveCount, rrp->CurveData);

			if (rrp->CurveCount == theItbl->refTblEntries) {	/* ready-to-use look-up table */
				memcpy (itblDat, rrp->CurveData, sizeof (*itblDat) * rrp->CurveCount);
			}
			else {
				PTErr = calcItblN (itblDat, theItbl->refTblEntries, rrp, interpMode);
				if (PTErr != KCP_SUCCESS) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
			}

			break;
		}
	}

    /* Replace grid tables:  */
	for (i = 0; i < 3; i++) {
		if (!IS_CHAN(theChan = theFut->chan[i])
			|| !IS_GTBL(theGtbl = theChan->gtbl)
			|| ((gtblDat[i] = theGtbl->refTbl) == NULL)) {	/* Get grid tables */
				PTErr = KCP_INCON_PT;
				goto ErrOut;
		}
	}

	calcGtbl3 (gtblDat, dim, mdata->matrix, offset);	/* without offset */

ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


/*-------------------------------------------------------------------------------
 *  makeMonotonic -- flatten reversals in data table
 *-------------------------------------------------------------------------------
 */
void
	makeMonotonic (KpUInt32_t count, mf2_tbldat_p table)
{
KpInt32_t		i;
mf2_tbldat_t	val;

     /* Check inputs:  */
	if ((table == (KpUInt16_p)NULL) || (count < 3)) {
		return;
	}

     /* Flatten from high end to low end, depending on polarity:  */
	if (table[0] <= table[count - 1]) {	/* globally non-decreasing */
		val = table[count - 1];
		for (i = count - 2; i >= 0; i--) {	/* from right to left */
			if (table[i] > val)	{	/* reversal? */
				table[i] = val;		/* flatten! */
			}
			else {					/* no reversal? */
				val = table[i];		/* update */
			}
		}
	}
	else {						/* globally decreasing */
		val = table[0];
		for (i = 1; i < (KpInt32_t)count; i++) {		/* from left to right */
			if (table[i] > val)	{	/* reversal? */
				table[i] = val;		/* flatten! */
			}
			else {					/* no reversal? */
				val = table[i];		/* update */
			}
		}
	}
}

