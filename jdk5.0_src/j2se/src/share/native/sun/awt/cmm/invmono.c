/*
 * @(#)invmono.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)invmono.c	1.10 98/11/04

	Contains:	makeInverseXformMono

	Written by:	Color Processor group

	COPYRIGHT (c) 1997-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include <string.h>
#include "makefuts.h"

/*---------------------------------------------------------------------------
 *  makeInverseXformMono -- make a fut of given gridsize from given TRC data
 *		for inverse transform (Lab -> gray); return status code
 *---------------------------------------------------------------------------
 */

PTErr_t
	makeInverseXformMono (	ResponseRecord_p	grayTRC,
							fut_p				theFut)
{
PTErr_t			PTErr = KCP_FAILURE;
fut_otbldat_p	otblDat;
ResponseRecord_t rrt;
KpInt32_t		futReturn;
fut_calcData_t	data;
double			gamma;
KpUInt16_t		rrpData[2] = { 0, RRECORD_DATA_SIZE -1 };
KpUInt16_t		*pCurveData = NULL;

	/* compute new grid table entries */
	data.chan = 0;
	if (!fut_calc_gtblEx (theFut->chan[0]->gtbl, fut_grampEx, &data)) {
		goto ErrOut0;
	}

	/* compute new output table entries */
	if (!fut_calc_otblEx (theFut->chan[0]->otbl, otblFunc, NULL)) {
		goto ErrOut0;
	}

	/* get address of the first output table */
	futReturn = fut_get_otbl (theFut, 0, &otblDat);
	if ((futReturn != 1) || (otblDat == (fut_otbldat_p)NULL)) {
		goto ErrOut0;
	}

	if (PARA_TYPE_SIG == grayTRC->TagSig)
	{
		pCurveData = (KpUInt16_p) allocBufferPtr (MFV_CURVE_TBL_ENT);	/* get memory for curve data */
		if (NULL == pCurveData) {
			PTErr =  KCP_NO_MEMORY;
			goto GetOut;
		}
		makeCurveFromPara (grayTRC->ParaFunction, grayTRC->ParaParams, pCurveData, MFV_CURVE_TBL_ENT);
		grayTRC->CurveCount = MFV_CURVE_TBL_ENT;
		grayTRC->CurveData = pCurveData;
	}
	/* setup the output table */
	switch (grayTRC->CurveCount) {
	case 0:
		/* setup the responseRecord struct */
		rrt.CurveCount = 2;
		rrt.CurveData = rrpData;

		/* make the output table */
		PTErr = calcOtblLN (otblDat, &rrt);
		break;

	case 1:
		gamma = (double)grayTRC->CurveData[0] / SCALEDOT8;
		if (gamma <= 0.0) {
			goto ErrOut0;
		}

		/* make the output table */
		PTErr = calcOtblL1 (otblDat, gamma);
		break;

	default:
		/* make the output table */
		makeInverseMonotonic (grayTRC->CurveCount, grayTRC->CurveData);

		PTErr = calcOtblLN (otblDat, grayTRC);
	}

GetOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;


ErrOut0:
	PTErr = KCP_SYSERR_0;
	goto GetOut;
}
