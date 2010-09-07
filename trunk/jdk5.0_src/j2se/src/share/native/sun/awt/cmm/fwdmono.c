/*
 * @(#)fwdmono.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)fwdmono.c	1.12 98/11/04

	Contains:	makeForwardXformMono

	COPYRIGHT (c) 1997-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include <string.h>
#include "makefuts.h"

#define FWD_MONO_OCHANS (3)

/*---------------------------------------------------------------------------
 *  makeForwardXformMono -- make a fut of given gridsize from given TRC data
 *		for forward transform (gray -> Lab); return status code
 *---------------------------------------------------------------------------
 */

PTErr_t
	makeForwardXformMono (	ResponseRecord_p	grayTRC,
							fut_p				theFut)
{
PTErr_t			PTErr = KCP_FAILURE;
KpInt32_t		futReturn, i1;
fut_otbldat_p	otblDat;
double			gamma;
fut_calcData_t	calcData;
KpUInt16_t		rrpData[2] = { 0, RRECORD_DATA_SIZE -1 };
ResponseRecord_t	rrt;
KpUInt16_t		*pCurveData = NULL;

	/* compute new table entries */
	calcData.chan = 0;				/* always uses 1st input chan */

	for (i1 = 0; i1 < FWD_MONO_OCHANS; i1++) {
		if (( ! IS_CHAN(theFut->chan[i1])) ||
			!fut_calc_gtblEx (theFut->chan[i1]->gtbl, fut_grampEx, &calcData) ||
			!fut_calc_otblEx (theFut->chan[i1]->otbl, otblFunc, NULL)) {
			goto ErrOut0;
		}
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
			return KCP_NO_MEMORY;
		}
		makeCurveFromPara (grayTRC->ParaFunction, grayTRC->ParaParams, pCurveData, MFV_CURVE_TBL_ENT);
		grayTRC->CurveCount = MFV_CURVE_TBL_ENT;
		grayTRC->CurveData = pCurveData;
	}
	/* setup the output table */
	switch (grayTRC->CurveCount)
	{
	case 0:
		/* setup the responseRecord struct */
		rrt.CurveCount = 2;
		rrt.CurveData = rrpData;

		/* make the output table */
		PTErr = calcOtblLSN (otblDat, &rrt);
		break;

	case 1:
		gamma = (double)grayTRC->CurveData[0] / SCALEDOT8;
		if (gamma <= 0.0) {
			goto ErrOut0;
		}

		/* make the output table */
		PTErr = calcOtblLS1 (otblDat, gamma);
		break;

	default:
		/* make the output table */
		makeMonotonic (grayTRC->CurveCount, grayTRC->CurveData);
		PTErr = calcOtblLSN (otblDat, grayTRC);
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
