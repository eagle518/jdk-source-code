/*
 * @(#)mat2fut.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)outmat.c	1.17 98/11/16

	Contains:	makeFutFromMatrix & makeXformFromMatrix

	Written by:	The Boston White Sox

	COPYRIGHT (c) 2001 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "kcms_sys.h"
#include <math.h>
#include <string.h>
#include "makefuts.h"

static PTErr_t makeXformFromMatrix (LPMATRIXDATA mdata, KpUInt32_t interpMode, KpInt32_p dim, fut_p theFut);
static PTErr_t calcItableN (mf2_tbldat_p table, KpInt32_t tableSize, ResponseRecord_p rrp, KpUInt32_t interpMode);
static void calcOtable0 (mf2_tbldat_p table);
static void calcOtable1 (mf2_tbldat_p table, double fwdgamma);
static PTErr_t calcOtableN (mf2_tbldat_p table, ResponseRecord_p rrp, KpUInt32_t interpMode);
static KpInt32_t initOTable (mf2_tbldat_p* tableP, double first, double last);

/*---------------------------------------------------------------------------------
 *  makeFutFromMatrix -- given a 3 x 3 matrix as part of a Lut8Bit or Lut16Bit
 *	profile for an input and/or output device, compute and return the equivalent PT
 *---------------------------------------------------------------------------------
 */

PTErr_t
	makeFutFromMatrix (	KpF15d16_p	matrix,
							ResponseRecord_p inRedTRC,
							ResponseRecord_p inGreenTRC,
							ResponseRecord_p inBlueTRC,
							ResponseRecord_p outRedTRC,
							ResponseRecord_p outGreenTRC,
							ResponseRecord_p outBlueTRC,
							KpUInt32_t gridsize,
							PTDataClass_t iClass,
							PTDataClass_t oClass,
							fut_p FAR*	theFut)
{
PTErr_t				PTErr;
ResponseRecord_p	inRR[MF_MATRIX_DIM], outRR[MF_MATRIX_DIM];
double				row0[MF_MATRIX_DIM], row1[MF_MATRIX_DIM], row2[MF_MATRIX_DIM], row3[MF_MATRIX_DIM];
double_p			rowp[MF_MATRIX_DIM+1];
MATRIXDATA			mdata;
KpInt32_t			dim[MF_MATRIX_DIM];

    /* Initialize ResponseRecords to do nothing (identity mapping):  */
	inRR[0] = inRedTRC;					/* set record pointers */
	inRR[1] = inGreenTRC;
	inRR[2] = inBlueTRC;

	outRR[0] = outRedTRC;				/* set record pointers */
	outRR[1] = outGreenTRC;
	outRR[2] = outBlueTRC;

    /* Form matrix (XYZ -> ABC):  */
	row0[0]	= (double)matrix[0] / SCALEFIXED;
	row0[1] = (double)matrix[1] / SCALEFIXED;
	row0[2] = (double)matrix[2] / SCALEFIXED;
	row1[0] = (double)matrix[3] / SCALEFIXED;
	row1[1] = (double)matrix[4] / SCALEFIXED;
	row1[2] = (double)matrix[5] / SCALEFIXED;
	row2[0] = (double)matrix[6] / SCALEFIXED;
	row2[1] = (double)matrix[7] / SCALEFIXED;
	row2[2] = (double)matrix[8] / SCALEFIXED;

	row3[0] = (double)matrix[9] / SCALEFIXED;
	row3[1] = (double)matrix[10] / SCALEFIXED;
	row3[2] = (double)matrix[11] / SCALEFIXED;

	rowp[0] = row0;					/* set row pointers */
	rowp[1] = row1;
	rowp[2] = row2;
	rowp[3] = row3;

    /* Construct matrix-data object:  */
	mdata.dim = 3;					/* always! */
	mdata.matrix = rowp;			/* set pointers */
	mdata.inResponse = inRR;
	mdata.outResponse = outRR;

	dim[0] = dim[1] = dim[2] = (KpInt32_t)gridsize;
	
	if (KCP_XYZ_PCS == iClass)
		iClass = KCP_VARIABLE_RANGE;
		
	*theFut = fut_new_empty (MF_MATRIX_DIM, dim, MF_MATRIX_DIM, iClass, oClass);	
	if (*theFut == NULL) {
	   return KCP_NO_MEMORY;
	}

    /* Make and return forward fut (XYZ -> ABC):  */
	PTErr = makeXformFromMatrix (&mdata, KCP_TRC_LAGRANGE4_INTERP, dim, *theFut);
	
	return PTErr;
}


/*----------------------------------------------------------------------------
 *  makeXformFromMatrix -- make a fut of given gridsize from given matrix data
 *----------------------------------------------------------------------------
 */
static PTErr_t
	makeXformFromMatrix (LPMATRIXDATA	mdata,
								KpUInt32_t		interpMode,
								KpInt32_p		dim,
								fut_p			theFut)
{
PTErr_t			PTErr = KCP_SUCCESS;
ResponseRecord_p	inRRp, outRRp;
KpInt32_t		i;
fut_itbl_p		theItbl;
fut_chan_p		theChan;
fut_gtbl_p		theGtbl;
fut_otbl_p		theOtbl;
mf2_tbldat_p	itblDat, prevItblDat, gtblDat[3], otblDat, prevOtblDat;
KpUInt16_t		prevGamma = 0, thisGamma, outCurveCount;
double			fwdgamma;
KpUInt16_t		*pCurveData = NULL;

	for (i = 0; i < MF_MATRIX_DIM; i++) {
		 /* Get ResponseRecord:  */
		inRRp = mdata->inResponse[i];
		if (NULL == inRRp) {
			break;				/* must only have output tables */
		}
		if (PARA_TYPE_SIG == inRRp->TagSig)
		{
			pCurveData = (KpUInt16_p) allocBufferPtr (MFV_CURVE_TBL_ENT);	/* get memory for curve data */
			if (NULL == pCurveData) {
				return KCP_NO_MEMORY;
			}
			makeCurveFromPara (inRRp->ParaFunction, inRRp->ParaParams, pCurveData, MFV_CURVE_TBL_ENT);
			inRRp->CurveCount = MFV_CURVE_TBL_ENT;
			inRRp->CurveData = pCurveData;
		}
		if ((inRRp->CurveCount > 0) && (inRRp->CurveData == (KpUInt16_p)NULL)) {
			PTErr = KCP_INCON_PT;
			goto ErrOut;
		}

		if (!IS_ITBL(theItbl = theFut->itbl[i])
			|| ((itblDat = theItbl->refTbl) == NULL)) {	/* Get input table */
				PTErr = KCP_INCON_PT;
				goto ErrOut;
		}		

		 /* Recompute input table:  */
		switch (inRRp->CurveCount) {
		case 0:	/* linear response */
			/* no-op:  leave ramps alone */
			break;

		case 1:	/* power law */
			thisGamma = inRRp->CurveData[0];
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
			makeMonotonic (inRRp->CurveCount, inRRp->CurveData);

			if (inRRp->CurveCount == theItbl->refTblEntries) {	/* ready-to-use look-up table */
				memcpy (itblDat, inRRp->CurveData, sizeof (*itblDat) * inRRp->CurveCount);
			}
			else {
				PTErr = calcItableN (itblDat, theItbl->refTblEntries, inRRp, interpMode);
				if (PTErr != KCP_SUCCESS) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
			}

			break;
		}
	}

	prevGamma = 0;
	for (i = 0; i < MF_MATRIX_DIM; i++) {
		 /* Get ResponseRecord:  */
		outRRp = mdata->outResponse[i];
		if (NULL == outRRp) {
			outCurveCount = 0;	/* must only have input tables, build extended range output tables*/
		}
		if ((outCurveCount > 0) && (outRRp->CurveData == (KpUInt16_p)NULL)) {
			PTErr = KCP_INCON_PT;
			goto ErrOut;
		}

		if (!IS_CHAN(theChan = theFut->chan[i])
			|| !IS_OTBL(theOtbl = theChan->otbl)
			|| ((otblDat = theOtbl->refTbl) == NULL)) {		/* Get output table */
				PTErr = KCP_INCON_PT;
				goto ErrOut;
		}
		if (theOtbl->refTblEntries != FUT_OUTTBL_ENT) {
			PTErr = KCP_INCON_PT;
			goto ErrOut;
		}
		 /* Recompute output table:  */
		switch (outCurveCount) {
		case 0:	/* linear response, with clipping */
			calcOtable0 (otblDat);
			break;
			
		case 1:	/* power law */
			thisGamma = outRRp->CurveData[0];
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
				calcOtable1 (otblDat, fwdgamma);
			}
			break;
			
		default:	/* look-up table of arbitrary length */
			makeMonotonic (outRRp->CurveCount, outRRp->CurveData);

			if (outRRp->CurveCount == theOtbl->refTblEntries) {	/* ready-to-use look-up table */
				memcpy (otblDat, outRRp->CurveData, sizeof (*otblDat) * outRRp->CurveCount);
			}
			else {
				PTErr = calcOtableN (otblDat, outRRp, interpMode);
				if (PTErr != KCP_SUCCESS) {
					PTErr = KCP_INCON_PT;
					goto ErrOut;
				}
			}

			break;
		}
	}
	/* Rescale given matrix by factor of 3 for extended range:  */
	for (i = 0; i < MF_MATRIX_DIM+1; i++) {
		KpInt32_t	j;

		for (j = 0; j < MF_MATRIX_DIM; j++) {
			mdata->matrix[i][j] /= 3.0;
		}
	}
	for (i = 0; i < MF_MATRIX_DIM; i++) {
		mdata->matrix[MF_MATRIX_DIM][i] += (1.0/3.0);
	}
    /* Replace grid tables:  */
	for (i = 0; i < MF_MATRIX_DIM; i++) {
		if (!IS_CHAN(theChan = theFut->chan[i])
			|| !IS_GTBL(theGtbl = theChan->gtbl)
			|| ((gtblDat[i] = theGtbl->refTbl) == NULL)) {	/* Get grid tables */
				PTErr = KCP_INCON_PT;
				goto ErrOut;
		}
	}
	calcGtbl3 (gtblDat, dim, mdata->matrix, mdata->matrix[3]);

ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}

/*-----------------------------------------------------------------------------------
 *  getNumParaParams -- return the number of parameters given the ICC function number
 *-----------------------------------------------------------------------------------
 */
KpInt32_t
getNumParaParams(KpUInt16_t	nFunction)
{
	KpInt32_t	nTblSize;

	switch(nFunction) {
	case 0:
		nTblSize = 1;
		break;

	case 1:
		nTblSize = 3;
		break;

	case 2:
		nTblSize = 4;
		break;

	case 3:
		nTblSize = 5;
		break;

	case 4:
		nTblSize = 7;
		break;

	default:
		nTblSize = -1;
	}
	return (nTblSize);
}

/*---------------------------------------------------------------------------------------------
 *  makeCurveFromPara -- create the curve data given a parametric function (defined by the ICC)
 *---------------------------------------------------------------------------------------------
 */
void
makeCurveFromPara (KpUInt16_t paraFunction, Fixed_p params, mab_tbldat_p curveTablePtr, KpInt32_t size)
{
	KpInt32_t		numParams, i, denominator;
	mab_tbldat_t	iResults;
	double			d, dResults, dParams[NUM_PARA_PARAMS];

	numParams = getNumParaParams(paraFunction);
	for (i = 0; i < numParams; i++)
	{
		dParams[i] = KpF15d16ToDouble(params[i]);
	}
	denominator = size - 1;
	for (i = 0; i < size; i++)
	{
		d = (double)i / denominator;
		switch(paraFunction)
		{
		case 0:
			dResults = pow(d, dParams[0]);
			break;

		case 1:
			if (d < -(dParams[2]/dParams[1]))
			{
				dResults = 0;
			} else {
				dResults = pow(dParams[1] * d + dParams[2], dParams[0]);
			}
			break;

		case 2:
			if (d < -(dParams[2]/dParams[1]))
			{
				dResults = dParams[3];
			} else {
				dResults = pow(dParams[1] * d + dParams[2], dParams[0]);
			}
			break;

		case 3:
			if (d < dParams[4])
			{
				dResults = dParams[3] * d;
			} else {
				dResults = pow(dParams[1] * d + dParams[2], dParams[0]);
			}
			break;

		case 4:
			if (d < dParams[4])
			{
				dResults = dParams[3] * d + dParams[6];
			} else {
				dResults = pow(dParams[1] * d + dParams[2], dParams[0]) + dParams[5];
			}
			break;

		default:
			break;
		}
		if (dResults < 0.0)
			dResults = 0.0;
		if (dResults > 1.0)
			dResults = 1.0;
			
		iResults = (mab_tbldat_t)(dResults * 65535.0);
		curveTablePtr[i] = iResults;
	}
}


/*---------------------  calcItableN  --------------------------------*/
/*
fut_itbldat_p		table;		pointer to input-table data
KpInt32_t			tableSize;	size of input table
ResponseRecord_p	rrp;		table of linearization data
KpUInt32_t 			interpMode;	interpolation mode to use
*/

static PTErr_t
	calcItableN (mf2_tbldat_p table, KpInt32_t tableSize, ResponseRecord_p rrp, KpUInt32_t interpMode)
{
PTErr_t			PTErr = KCP_SUCCESS;
KpUInt32_t		length;
KpUInt16_p		data;
KpInt32_t		count, ix, hint = 1;
mf2_tbldat_t	lval;
double			val, x, x1, frac, nument_inv_li, nument_inv_l4;
xfer_t			Xfer;
KpBool_t		decreasing;
KpUInt16_t		*pCurveData = NULL;

	/* Check input and initialize:  */
	if (rrp == NULL) return KCP_BAD_ARG;
	if (table == NULL)	return KCP_BAD_ARG;
	
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
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}
	if (data == NULL) {
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}

	if (data[0] > data[length-1]) {
		decreasing = KPTRUE;
	} else {
		decreasing = KPFALSE;
	}
	   
	PTErr = init_xfer (&Xfer, rrp);
	if (PTErr != KCP_SUCCESS) {
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}
	
	PTErr = set_xfer (&Xfer, 0, 1);
	if (PTErr != KCP_SUCCESS) {
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}

	nument_inv_li = (double)(length - 1) / (double)(tableSize - 1);
	nument_inv_l4 = 1.0 / (double)(tableSize - 1);
	
	if (length < 32)
		interpMode = KCP_TRC_LINEAR_INTERP;	/* msm xxx THIS NEEDS TO BE FIXED IT SHOULD BE KCP_TRC_LAGRANGE4_INTERP!!! */

     /* Loop over regular entries, converting index to floating-point variable:  */
	for (count = 0; count < tableSize; count++) {
		
		/* Compute linearized value by interpolating in transfer table:  */
		x1 = (double)count * nument_inv_l4;	/* in [0, 1] */

		switch (interpMode) {
		case KCP_TRC_LINEAR_INTERP:

	 	    /* Compute linearized value by interpolating in transfer table:  */
			x = (double)count * nument_inv_li;		/* in [0, length - 1] */
			ix = (KpUInt32_t)x;						/* integer part */
			if (ix >= (length - 1)) {				/* off end of data table */
				val = (double)data[length - 1] / SCALEDOT16;	/* take last value */
			}
			else {								/* within data table, interpolate */
				frac = x - (double)ix;			/* fractional part */
				val = (double)data[ix] + frac * ((double)data[ix + 1] - (double)data[ix]);
				val /= SCALEDOT16;
			}						/* in [0, 1] */

			break;
		
		case KCP_TRC_LAGRANGE4_INTERP:
	
/* !!!!  LAGRANGIAN NOT WORK IF THE SOURCE DATA IS NON-MONOTONIC OR HAS FLAT REGIONS !!!! */
			val = xfer (&Xfer, x1, &hint);		/* in [0, 1] */

			break;
		
		default:
			PTErr = KCP_BAD_ARG;
			goto ErrOut;
		}

		if (length < SLOPE_COUNT) {
			if (decreasing == KPTRUE) {
				val = MIN (val, (1-x1)*SLOPE_LIMIT);	/* limit the decreasing slope */
			} else {
				val = MAX (x1/SLOPE_LIMIT, val);		/* limit the increasing slope */
			}
		}
	
		lval = QUANT_MF2 (val, MF2_TBL_MAXVAL);     /* Rescale, clip, and quantize for input table:  */
		table[count] = lval;
	}
	
ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


/*---------------------------------------------------------------------------
 *  calcOtbl0, calcOtbl1, calcOtableN -- calculate an output table
 *		to clip extended range, gamma-correct RGB outputs according to 
 *		specified algorithm (linear, power law, or transfer table),
 *		and requantize.
 *---------------------------------------------------------------------------
 */

static void
	calcOtable0 (mf2_tbldat_p table)
{
KpInt32_t		count;
double			p, nument_inv;
mf2_tbldat_t	odata;

	if (table == (mf2_tbldat_p)NULL) return;	/* just don't crash! */

	count = initOTable (&table, 0.0, 1.0);

	nument_inv = 1.0 / (double) (FUT_OUTTBL_ENT -1);

	/* middle third is non-clipped data */
	for (; count < FUT_OUTTBL_ENT -1; count += 3) {
	    /* Treat otbl index as output from a gtbl scaled to FUT_OUTTBL_ENT -1:  */
		p = (double)count * nument_inv;		/* in [0, 1] */

		odata = QUANT_MF2 (p, MF2_TBL_MAXVAL);	/* Rescale to max */

		*(table++) = odata;
	}
}


static void
	calcOtable1 (mf2_tbldat_p table, double fwdgamma)
{
KpInt32_t		count;
double			p, val, nument_inv;
mf2_tbldat_t	odata;

    /* Check input parameters:  */
	if (table == (mf2_tbldat_p)NULL) return;	/* just don't crash! */
	if (fwdgamma == 0.0) return;

	if (fwdgamma == 1.0) {	/* trivial */
	   calcOtbl0 (table);
	   return;
	}

	count = initOTable (&table, 0.0, 1.0);

	nument_inv = 1.0 / (double) (FUT_OUTTBL_ENT -1);

	/* middle third */
	for (; count < FUT_OUTTBL_ENT -1; count += 3) {

	     /* Treat otbl index as output from a gtbl scaled to 4095:  */
		p = (double)count * nument_inv;	/* in [0, 1] */

	     /* compute correction from power law:  */
		val = (double)pow (p, fwdgamma);

		val = MAX (p/SLOPE_LIMIT, val);		/* limit the slope */

		odata = QUANT_MF2 (val, MF2_TBL_MAXVAL);	/* Rescale to max */

		*table = odata;						/* in [0, norm - 1] */

		table++;
	}
}


static PTErr_t
	calcOtableN (mf2_tbldat_p table, ResponseRecord_p rrp, KpUInt32_t interpMode)
{
PTErr_t			PTErr = KCP_SUCCESS;
KpUInt32_t		length;
KpUInt16_p		data;
KpInt32_t		count, hint = 1;
double			val, nument_inv;
xfer_t			Xfer;
mf2_tbldat_t	odata;
KpBool_t		decreasing;
double			p, first, last;
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
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}
	if (data == NULL) {
		PTErr = KCP_BAD_ARG;
		goto ErrOut;
	}

	if (data[length - 1] == data[0]) {
		PTErr = KCP_BAD_ARG;	/* empty domain */
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

	switch (interpMode) {		
	case KCP_TRC_LAGRANGE4_INTERP:
	
/* !!!!  LAGRANGIAN DOES NOT WORK IF THE SOURCE DATA IS NON-MONOTONIC OR HAS FLAT REGIONS !!!! */

		PTErr = init_xfer (&Xfer, rrp);
		if (PTErr != KCP_SUCCESS) {
			PTErr = KCP_BAD_ARG;
			goto ErrOut;
		}
		
		PTErr = set_xfer (&Xfer, 0, 1);
		if (PTErr != KCP_SUCCESS) {
			PTErr = KCP_BAD_ARG;
			goto ErrOut;
		}

		first = (double) data[0] / MF2_TBL_MAXVAL;
		last = (double) data[length-1] / MF2_TBL_MAXVAL;		
		count = initOTable (&table, first, last);
	
		/* middle third */
		for (; count < FUT_OUTTBL_ENT -1; count += 3) {

		     /* Treat otbl index as output from a gtbl scaled to 4095:  */
			p = (double)count * nument_inv;		/* in [0, 1] */
	
		     /* Clip and compute correction from inverse power law:  */
			val = xfer (&Xfer, p, &hint);
	
			if (length < SLOPE_COUNT) {
				if (decreasing == KPTRUE) {
					val = MIN (val, (1-p)*SLOPE_LIMIT);		/* limit the decreasing slope */
				}
				else {
					val = MAX (p/SLOPE_LIMIT, val);			/* limit the increasing slope */
				}
			}

			odata = QUANT_MF2 (val, MF2_TBL_MAXVAL);	/* Rescale to max */

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

	/* first third */
	first = QUANT_MF2 (firstD, MF2_TBL_MAXVAL);
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
	last = QUANT_MF2 (lastD, MF2_TBL_MAXVAL);
	for (; count < 2 * FUT_OUTTBL_ENT; count += 3) {
		*(table++) = last;
	}

	return retCount;
}



