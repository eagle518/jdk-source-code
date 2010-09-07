/*
 * @(#)calcitbl.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	calcitbl.c

	Contains:	PTCreateTRC, calcItblN

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include "makefuts.h"


typedef struct fData_s {
	fut_calcData_t	std;
	KpFloat32_t		gamma;
} fData_t, FAR* fData_p;

/*----------------------------------------------------------------------------
 *  calcItblN -- calculate an input table
 *		to linearize RGB inputs according to specified algorithm (power law or transfer table)
 *----------------------------------------------------------------------------
 */

/* Compute linearized value by power law */
static double
	ipowfunc (double x, fut_calcData_p data)
{
double	val;

	val = (double)pow (x, ((fData_p) data)->gamma);		/* in [0, 1] */
	val = MAX (x/SLOPE_LIMIT, val);		/* limit the slope */

	return (val);
}


PTErr_t
	PTCreateTRC (KpUInt16_p TRC, KpFloat32_t gamma)
{
PTErr_t		PTErr;
fut_itbl_p	itbl;
fData_t		fDataL;

	if (TRC == NULL) return KCP_BAD_PTR;

	fDataL.gamma = gamma;
	
	itbl = fut_new_itblEx (KCP_REF_TABLES, KCP_FIXED_RANGE, 2, ipowfunc, &fDataL.std);
	if (itbl == NULL) {
		return KCP_NO_MEMORY;
	}
	
	if (itbl->refTblEntries != FUT_INPTBL_ENT) {
		PTErr = KCP_PTERR_0;
	}
	else {
		KpMemCpy (TRC, itbl->refTbl, itbl->refTblEntries * sizeof(mf2_tbldat_t));
		PTErr = KCP_SUCCESS;
	}
	
	fut_free_itbl (itbl);
	
	return PTErr;
}


/*---------------------  calcItblN  --------------------------------*/
/*
fut_itbldat_p		table;		pointer to input-table data
KpInt32_t			tableSize;	size of input table
ResponseRecord_p	rrp;		table of linearization data
KpUInt32_t 			interpMode;	interpolation mode to use
*/

PTErr_t
	calcItblN (mf2_tbldat_p table, KpInt32_t tableSize, ResponseRecord_p rrp, KpUInt32_t interpMode)
{
PTErr_t			PTErr = KCP_SUCCESS;
KpUInt32_t		length;
KpUInt16_p		data;
KpInt32_t		count, ix, hint = 1;
mf2_tbldat_t	lval;
double			val, x, x1, frac, nument_inv_li, nument_inv_l4, dblMaxVal = MF2_TBL_MAXVAL;
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
	
		lval = QUANT_MF2 (val, dblMaxVal);     /* Rescale, clip, and quantize for input table:  */
		table[count] = lval;
	}
	PTErr = KCP_SUCCESS;
	
ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}
