/*
 * @(#)eval.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)eval.c	1.5 99/03/04

	Contains:	evaluation of a multiple channels of a fut.

	COPYRIGHT (c) 1991-1998 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "fut_util.h"
#include "kcptmgr.h"
#include "kcpcache.h"


/* evalFut evaluates multiple channels at a set of n points specified by
 * a collection of input arrays, and places the results into an output array.
 * The number and position of output arrays are determined by the supplied omask.
 * The number and position of input arrays are determined by the first output chan's imask.
 *
 * dataType specifies the size of the evaluation data
 * as KCM_UBYTE, KCM_USHORT_12, or KCM_USHORT.
 */


KpInt32_t
	evaluateFut (	fut_p				evalFut,
					KpUInt32_t			omask,
					KpUInt32_t			dataType,
					KpInt32_t			n,
					KpGenericPtr_t FAR*	in,
					KpGenericPtr_t FAR*	out)
{
PTErr_t			error;
PTRefNum_t		PTRefNum;
KpInt32_t		i1, i2, imask, stride, hasOTbl;
PTTable_p		PTTableP;
evalControl_t	evalControl;

	if (omask == 0) {
		return 1;		/* nothing to do */
	}

	error = registerPT (NULL, NULL, &PTRefNum);
	if (error != KCP_SUCCESS) {
		return 0;
	}

	PTTableP = lockPTTable (PTRefNum);

	PTTableP->dataP = evalFut;						/* define the fut */
	PTTableP->data = getHandleFromPtr (evalFut);

	for (i1 = 0, hasOTbl = 0; i1 < FUT_NOCHAN; i1++) {
		if ((FUT_BIT(i1) & omask) != 0) {
			imask = evalFut->chan[i1]->imask;		/* use imask from last output channel */
			if ((evalFut->chan[i1]->otbl != NULL) && (evalFut->chan[i1]->otbl->refTbl != NULL)) {
				hasOTbl = 1;						/* at least one otbl present */
			}
		}
	}
		

	/* Initialize the evaluation control structure */
	evalControl.callBack = NULL;
	evalControl.evalFunc = evalTh1gen;

	if (hasOTbl != 1) {
		switch (imask) {
		case FUT_XYZ:
			evalControl.evalFunc = evalTh1i3oXd16n;
			break;
		
		case FUT_XYZT:
			evalControl.evalFunc = evalTh1i4oXd16n;
			break;
		
		default:
			break;
		}
	}

	evalControl.nFuts = 1;
	evalControl.evalList = &PTTableP;
	evalControl.ioMaskList[0] = FUT_IN(imask) | FUT_OUT(omask);
	evalControl.optimizedEval = 0;
	evalControl.compatibleDataType = 1;
	evalControl.evalDataTypeI = dataType;
	evalControl.evalDataTypeO = dataType;
	evalControl.imageLines = 1;
	evalControl.imagePels = n;

	if (dataType == KCM_UBYTE) {
		stride = sizeof (KpUInt8_t);
	}
	else {
		stride = sizeof (KpUInt16_t);
	}

	/* set up input address/stride arrays */
	for (i1 = 0, i2 = 0; i1 < FUT_NICHAN; i1++) {
		if ((imask & FUT_BIT(i1)) != 0) {
			evalControl.inputData[i1].pI = in[i2];
			evalControl.inPelStride[i1] = stride;
			evalControl.inLineStride[i1] = n * stride;
			i2++;
		}
		else {
			evalControl.inputData[i1].pI = NULL;
			evalControl.inPelStride[i1] = 0;
			evalControl.inLineStride[i1] = 0;
		}
	}

	/* set up output address/stride arrays */
	for (i1 = 0, i2 = 0; i1 < FUT_NOCHAN; i1++) {
		if ((omask & FUT_BIT(i1)) != 0) {
			evalControl.outputData[i1].pI = out[i2];
			evalControl.outPelStride[i1] = stride;
			evalControl.outLineStride[i1] = n * stride;
			i2++;
		}
		else {
			evalControl.outputData[i1].pI = NULL;
			evalControl.outPelStride[i1] = 0;
			evalControl.outLineStride[i1] = 0;
		}
	}

	error = initEvalTables (&evalControl);	/* set up tables */
	if (error == KCP_SUCCESS) {

		error = evalImageMP (&evalControl);	/* evaluate! */
	}

	unlockPTTable (PTRefNum);
	freeEvalTables (PTRefNum);
	deletePTTable (PTRefNum);

	return (error == KCP_SUCCESS) ? 1 : 0; 
}
