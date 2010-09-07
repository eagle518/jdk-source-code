/*
 * @(#)xfers.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)xfers.c	1.15 98/11/04

	Contains:	init_xfer, set_xfer, xfer

	Written by:	The Boston White Sox

	COPYRIGHT (c) 1993-1998 Eastman Kodak Company
	As  an unpublished  work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <math.h>
#include "kcmsos.h"
#include "fut.h"
#include "makefuts.h"

/*-------------------------------------------------------------------
 *  init_xfer -- initialize the transfer tables by transferring coarse
 *			control points from an ICC ResponseRecord
 *			and interpolating the fine tables from them;
 *			returns +1 for success, -1 for failure to
 *			allocate memory for coarse table
 *-------------------------------------------------------------------
 */
PTErr_t
	init_xfer (	xfer_p				xferp,
				ResponseRecord_p	rrp)
{
PTErr_t		PTErr = KCP_SUCCESS;
double		val;			/* input variables */
KpInt32_t	numcoarse;		/* number of input control points */
double_p	coarse[2];		/* storage for control points */
KpInt32_t	i;			/* control-point index */
KpInt32_t	hint;
KpUInt16_t	*pCurveData = NULL;

     /* Verify inputs:  */
	if (xferp == (xfer_p)NULL)	return KCP_SYSERR_0;
	if (rrp == (ResponseRecord_p)NULL)	return KCP_SYSERR_0;
	
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
	if (rrp->CurveCount < 2) {
		PTErr = KCP_SYSERR_0;
		goto ErrOut;
	}
	if (rrp->CurveData == (KpUInt16_p)NULL) {
		PTErr = KCP_SYSERR_0;
		goto ErrOut;
	}
	   
     /* Allocate space for coarse tables:  */
	numcoarse = rrp->CurveCount - 1;	/* skip zero entry to avoid infinity in logarithm */
	coarse[0] = (double_p)ALLOC (numcoarse, sizeof(double));
	if (coarse[0] == NULL)
	{
		PTErr = KCP_NO_MEMORY;
		goto ErrOut;
	}
	coarse[1] = (double_p)ALLOC (numcoarse, sizeof(double));
	if (coarse[1] == NULL)
	{
	   DALLOC (coarse[0]);	/* release storage */
		PTErr = KCP_NO_MEMORY;
		goto ErrOut;
	}

     /* Build coarse tables from ResponseRecord:  */
	for (i = 0; i < numcoarse; i++)
	{
	   val = (double)(i + 1) / (double)numcoarse;	/* skip zero to avoid infinite log */
	   coarse[0][i] = -log10 (val);
	   val = (double)rrp->CurveData[i + 1] / SCALEDOT16;
	   val = MAX (val, 1.0e-12);			/* clip to avoid infinite log */
	   coarse[1][i] = -log10 (val);
	}

     /* Build fine tables by interpolating in coarse tables:  */
	hint = 1;
	for (i = 0; i < NUMFINE; i++)				/* spaced code values */
	{
	   double	code;

	   code = (double)i * 2.4 / (double)(NUMFINE - 1);	/* equally spaced in [0, 2.4] */
	   xferp->nonlinear[i] = code;
	   xferp->linear[i] = f4l (code, coarse[0], coarse[1], numcoarse, &hint);
	}

     /* Delete coarse tables:  */
	DALLOC (coarse[0]);
	DALLOC (coarse[1]);

ErrOut:
	if (NULL != pCurveData) {
		freeBufferPtr (pCurveData);
	}
	return PTErr;
}


/*-------------------------------------------------------------------
 *  set_xfer -- select source and destination channels for transfer;
 *		returns +1 for success, -2 for null xferp, -3 for bad
 *		channel arguments
 *-------------------------------------------------------------------
 */
PTErr_t
	set_xfer (	xfer_p		xferp,
				KpInt32_t	source,
				KpInt32_t	dest)
{
     /* Validate arguments:  */
	if (xferp == (xfer_p)NULL)	return KCP_SYSERR_0;
	if ((source < 0) || (source > 1))	return KCP_SYSERR_0;
	if ((dest < 0) || (dest > 1))	return KCP_SYSERR_0;

     /* Set channels:  */
	xferp->from = (source == 0)	? xferp->nonlinear : xferp->linear;
	xferp->to = (dest == 0) ? xferp->nonlinear : xferp->linear;

	return KCP_SUCCESS;
}


/*-------------------------------------------------------------------
 *  xfer -- transfer function interpolated in given table
 *-------------------------------------------------------------------
 */
double
	xfer (	xfer_p		xferp,
			double		inval,
			KpInt32_p	hint)
{
double	indens, outdens;	/* densities */

	if (inval <= 0.0) return 0.0;
	if (inval >= 1.0) return 1.0;

     /* Interpolate in density space:  */
	inval = MAX (inval, 1.0e-12);	/* Dmax = 12 */
	indens = -log10 (inval);
	outdens = f4l (indens, xferp->from, xferp->to, NUMFINE, hint);
	return pow (0.10, outdens);
}
