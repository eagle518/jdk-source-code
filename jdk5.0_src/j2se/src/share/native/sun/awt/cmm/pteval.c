/*
 * @(#)pteval.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)pteval.c	2.56 99/03/04

	Contains:	Handle function evaluations using PTs

    COPYRIGHT (c) Eastman Kodak Company, 1991-1999
    As an unpublished work pursuant to Title 17 of the United
    States Code.  All rights reserved.
 */

#include "attrib.h"
#include "kcptmgr.h"
#include "fut_util.h"

#if defined (KCP_ACCEL)
#include "ctelib.h"

#define	FASTER_IN_SW 8000
#endif

static KpUInt32_t calcChanMask (KpInt32_t, KpInt32_t, PTCompDef_p);
static KpInt32_t calcChans (KpUInt32_t);
static PTErr_t setupEvalList (KpUInt32_t, PTTable_p*, KpUInt32_p, PTEvalDTPB_p, KpUInt32_p);
static PTErr_t getDataBytes (KpInt32_t,	KpInt32_p);


PTErr_t
	 PTEval (	PTRefNum_t		PTRefNum,
				PTEvalPB_p		evalDef,
				PTEvalTypes_t	evalID,
				KpInt32_t		devNum,
				KpInt32_t		aSync,
				opRefNum_p		opRefNum,
				PTProgress_t	progress)
{
PTErr_t			PTErr;
callBack_t		callBack;

	if (devNum) {}
	if (aSync) {}
	if (opRefNum) {}
	
	callBack.progressFunc = progress;

	PTErr = PTEvalP (PTRefNum, evalDef, evalID, 0, 0, NULL, &callBack);

	return (PTErr);
}

PTErr_t
	 PTEvalP (	PTRefNum_t		PTRefNum,
				PTEvalPB_p		evalDef,
				PTEvalTypes_t	evalID,
				KpInt32_t		devNum,
				KpInt32_t		aSync,
				opRefNum_p		opRefNum,
				callBack_p		callBack)
{
PTEvalDTPB_t	evalDTDef;
PTCompDef_t 	imageInput[FUT_NICHAN];
PTCompDef_t 	imageOutput[FUT_NOCHAN];
KpInt32_t		i;
PTErr_t			PTErr;

	if (devNum) {}
	if (aSync) {}
	if (opRefNum) {}

	/* valid PTEvalDTPB_p? */
	if (evalDef == NULL) goto ErrOut1;
	if (evalDef->input == NULL) goto ErrOut1;
	if (evalDef->output == NULL) goto ErrOut1;
	if (evalDef->nInputs > FUT_NICHAN) goto ErrOut3;
	if (evalDef->nOutputs > FUT_NOCHAN) goto ErrOut3;

	evalDTDef.nPels = evalDef->nPels;
	evalDTDef.nLines = evalDef->nLines;
	evalDTDef.nInputs = evalDef->nInputs;
	evalDTDef.input = imageInput;
	evalDTDef.dataTypeI = KCM_UBYTE;
	
	for (i = 0; i < evalDTDef.nInputs; i++) {
		evalDTDef.input[i] = evalDef->input[i];
	}

	evalDTDef.nOutputs = evalDef->nOutputs;
	evalDTDef.output = imageOutput;
	evalDTDef.dataTypeO = KCM_UBYTE;

	for (i = 0; i < evalDTDef.nOutputs; i++) {
		evalDTDef.output[i] = evalDef->output[i];
	}

	PTErr = PTEvaluate (PTRefNum, &evalDTDef, evalID, 0, 0, NULL, callBack);

ErrOut0:
	return (PTErr);


ErrOut1:
	PTErr = KCP_BAD_PTR;
	goto ErrOut0;

ErrOut3:
	PTErr = KCP_INVAL_EVAL;
	goto ErrOut0;
}


PTErr_t
	PTEvalDT (	PTRefNum_t		PTRefNum,
				PTEvalDTPB_p	evalDef,
				PTEvalTypes_t	evalID,
				KpInt32_t		devNum,
				KpInt32_t		aSync,
				opRefNum_p		opRefNum,
				PTProgress_t	progress)
{
PTErr_t			PTErr;
callBack_t		callBack;

	if (devNum) {}
	if (aSync) {}
	if (opRefNum) {}

	callBack.progressFunc = progress;

	PTErr = PTEvaluate (PTRefNum, evalDef, evalID, 0, 0, NULL, &callBack);

	return (PTErr);
}


PTErr_t
	PTEvaluate (	PTRefNum_t		PTRefNum,
					PTEvalDTPB_p	evalDef,
					PTEvalTypes_t	evalID,
					KpInt32_t		devNum,
					KpInt32_t		aSync,
					opRefNum_p		opRefNum,
					callBack_p		callBack)
{
PTErr_t			PTErr;
PTEvalDTPB_t	lEvalDef;
PTCompDef_t		thisInput [FUT_NICHAN], thisOutput [FUT_NOCHAN];
PTRefNum_t		PTList [MAX_PT_CHAIN_SIZE];
PTTable_p		evalList [MAX_PT_CHAIN_SIZE], PTTableP;
PTTable_p*		listStart;
KpInt32_t		theSerialCount, i1, i2, i3, i4, nOutputs, nFuts, PTcount;
KpUInt32_t		tempMemNeeded, oMask, ioMaskList [MAX_PT_CHAIN_SIZE];
PTImgAddr_t		addr;
#if defined (KCP_ACCEL)
PTEvalTypes_t	evaluator;
KpInt32_t		numEvals;
#endif

	if (devNum) {}
	if (aSync) {}
	if (opRefNum) {}

#if defined (KCP_MACPPC_MP)
	KCPInitializeMP ();
#endif

	PTErr = getPTStatus (PTRefNum);			/* must be an active or serial PT */
	if ((PTErr != KCP_PT_ACTIVE) && (PTErr != KCP_SERIAL_PT)) {
		goto ErrOut0;
	}

#if defined (KCP_ACCEL)
	PTErr = GetEval (evalID, &evaluator);	/* get an evaluator */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut0;
	}
#endif

	/* valid PTEvalDTPB_p? */
	if (evalDef == NULL) goto ErrOut1;
	if (evalDef->input == NULL) goto ErrOut1;
	if (evalDef->output == NULL) goto ErrOut1;
	if (evalDef->nInputs > FUT_NICHAN) goto ErrOut3;
	if (evalDef->nOutputs > FUT_NOCHAN) goto ErrOut3;

	/* set up the local evaluation structures */
	/* set input and output to NULL */
	/* this preserves the channel position while allowing */
	/* the number of channels to indicate the number of valid addresses */
	/* this is needed to keep both the CTE and SW evaluations happy */
	for (i1 = 0; i1 < FUT_NICHAN; i1++) {
		thisInput[i1].pelStride = 0;
		thisInput[i1].lineStride = 0;
		thisInput[i1].addr = NULL;
	} 

	lEvalDef.nPels = evalDef->nPels;
	lEvalDef.nLines = evalDef->nLines;

	lEvalDef.nInputs = FUT_NICHAN;
	lEvalDef.dataTypeI = evalDef->dataTypeI;
	lEvalDef.input = thisInput;
	for (i1 = 0; i1 < evalDef->nInputs; i1++) {
		lEvalDef.input[i1].pelStride	= evalDef->input[i1].pelStride;
		lEvalDef.input[i1].lineStride	= evalDef->input[i1].lineStride;
		lEvalDef.input[i1].addr			= evalDef->input[i1].addr;
	} 

	/* output addresses are loaded in the evaluation loop, no need to do anything here */

	/* clear the PT and evaluation lists, just for clarity */
	for (i1 = 0; i1 < MAX_PT_CHAIN_SIZE; i1++) {
		PTList[i1] = NULL;
		evalList[i1] = NULL;
	}

	/* get the list of PTs which we must actually evaluate */
	PTErr = resolvePTData (PTRefNum, &theSerialCount, PTList);

	/* set up list of futs through which the image is evaluated */
	for (i1 = 0; i1 < theSerialCount; i1++) {
		PTTableP = lockPTTable (PTList[i1]);	/* lock tables while evaluating */
		evalList[i1] = PTTableP;
	}

	/* initialize the evaluation list */
	PTErr = setupEvalList (theSerialCount, evalList, ioMaskList, evalDef, &tempMemNeeded);
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut2;
	}

	/* if temporary memory is not needed, */
	/* then evaluate a full image at a time until */
	/* the image has been processed through all futs */
	/* if temporary memory is needed, */
	/* then this level processes the image just once */
	/* and a lower level processes the image through all futs */
	if (tempMemNeeded == 0) {
		PTcount = theSerialCount;
	}
	else {
		PTcount = 1;
	}

	initProgressPasses (PTcount, callBack);

	/* process the image through each fut in the list */ 
	for (i1 = 0; i1 < PTcount; i1++) {

		/* set up the output data addresses */
		/* use io mask to order output channels properly */
		if (tempMemNeeded == 1) {
			nFuts = theSerialCount;					/* this many futs to evaluate */
			listStart = &evalList[0];
			oMask = FUT_OMASK(ioMaskList[nFuts-1]);	/* use mask of last fut */
		}
		else {
			nFuts = 1;								/* evaluate one fut */
			listStart = &evalList[i1];
			oMask = FUT_OMASK(ioMaskList[i1]);		/* use mask of this fut */
		}

		/* initialize the output data structures */
		lEvalDef.nOutputs = FUT_NOCHAN;
		lEvalDef.dataTypeO = evalDef->dataTypeO;
		lEvalDef.output = thisOutput;

		for (i2 = 0; i2 < FUT_NOCHAN; i2++) {
			thisOutput[i2].pelStride = 0;
			thisOutput[i2].lineStride = 0;
			thisOutput[i2].addr = NULL;
		} 

		/* set the output channel addresses */
		if (i1 == (PTcount -1)) {		/* last, just use supplied stuff */
			for (i2 = 0, nOutputs = 0; i2 < evalDef->nOutputs; i2++) {
				lEvalDef.output[i2].pelStride	= evalDef->output[i2].pelStride;
				lEvalDef.output[i2].lineStride	= evalDef->output[i2].lineStride;
				lEvalDef.output[i2].addr		= evalDef->output[i2].addr;

				nOutputs++;		/* count actual outputs */
			}

			getDataBytes (evalDef->dataTypeO, &i3);	/* get output data size */
			if (i3 == 0) {
				nOutputs = 3;
			}
		}
		else {
			for (i2 = oMask, i3 = 0, i4 = 0, nOutputs = 0; i2 != 0; i2 >>= 1, i3++) {
				if ((i2 & 1) == 1) {	/* this output channel is needed */
					while ((addr = evalDef->output[i4].addr) == NULL) {
						i4++;		/* get next available output channel */
					}

					if (i4 > evalDef->nOutputs) {
						PTErr = KCP_PTERR_4;				/* programming error */
						goto ErrOut2;
					}

					lEvalDef.output[i3].pelStride	= evalDef->output[i4].pelStride;
					lEvalDef.output[i3].lineStride	= evalDef->output[i4].lineStride;
					lEvalDef.output[i3].addr		= addr;

					i4++;			/* next output address */
					nOutputs++;		/* count actual outputs */
				}
			}
		}

#if defined (KCP_ACCEL)
		/* if there are less than FASTER_IN_SW evaluations, it's faster to do it in software. */
		numEvals = lEvalDef.nPels * lEvalDef.nLines * nOutputs;
		
		if ((numEvals < FASTER_IN_SW) || (tempMemNeeded == 1)) {
			evaluator = KCP_EVAL_SW;
		}
		
		switch (evaluator) {
		case KCP_EVAL_SW:		/* evaluate in software */

software_evaluation:
#endif

			PTErr = PTEvalSeq (nFuts, listStart, &ioMaskList[i1], &lEvalDef, callBack);
			if (PTErr != KCP_SUCCESS) {
				goto ErrOut2;
			}

#if defined (KCP_ACCEL)
			break;

		case KCP_EVAL_CTE:	/* evaluate using the NFE */
		{
		PTRefNum_t		thePTRefNum;
		KpHandle_t		PTData;

			thePTRefNum = listStart[0]->refNum;			/* get the PT reference number */

			PTData = getPTData (thePTRefNum);	/* get the transform data */

			PTErr = PT_eval_cteDT (PTData, &lEvalDef, 0, 0, callBack);

			if ((PTErr == KCP_CTE_GRID_TOO_BIG) || (PTErr == KCP_CTE_NOT_ATTEMPTED)) {
				goto software_evaluation;
			}
			
			break;
		}

		default:
			PTErr = KCP_INVAL_EVAL;

			break;
		}
#else
		if (evalID) {}	/* unreferenced formal parameter */
#endif
				
		/* output for this PT is input for next PT */
		lEvalDef.nInputs = lEvalDef.nOutputs;
		lEvalDef.dataTypeI = lEvalDef.dataTypeO;

		for (i2 = 0; i2 < lEvalDef.nInputs; i2++) {
			lEvalDef.input[i2].pelStride	= lEvalDef.output[i2].pelStride;
			lEvalDef.input[i2].lineStride	= lEvalDef.output[i2].lineStride;
			lEvalDef.input[i2].addr			= lEvalDef.output[i2].addr;
		}
	}


ErrOut2:
	/* unlock the PTs used for evaluation */
	for (i1 = 0; i1 < theSerialCount; i1++) {
		unlockPTTable (PTList[i1]);
	}

ErrOut0:
	return (PTErr);


ErrOut1:
	PTErr = KCP_BAD_PTR;
	goto ErrOut0;

ErrOut3:
	PTErr = KCP_INVAL_EVAL;
	goto ErrOut0;
}


/* set up the fut evaluation list */
/* Determine whether or not temporary memory is needed */
static PTErr_t
	setupEvalList (	KpUInt32_t		numPTs,
					PTTable_p*		evalList,
					KpUInt32_p		ioMaskList,
					PTEvalDTPB_p	evalDef,
					KpUInt32_p		tempMemNeeded)
{
PTErr_t		PTErr = KCP_SUCCESS;
KpInt32_t	i1, i2;
KpInt32_t	finalOutputs, nOutputs, maxOutputs;
KpUInt32_t	thisOmask, thisImask;
KpInt32_t	sizeInData, sizeOutData;
fut_chan_p	futChan;

	thisOmask = calcChanMask (evalDef->dataTypeO, evalDef->nOutputs, evalDef->output);	/* calculate final output mask */
	finalOutputs = calcChans (thisOmask);		/* and number of output channels */
	
	/* does the fut have those outputs? */
	if ((FFUTP(evalList[numPTs -1]->data)->iomask.out & thisOmask) != thisOmask) {
		PTErr = KCP_INVAL_EVAL;
		goto ErrOut;
	}

	nOutputs = finalOutputs;	/* # outputs of fut at the end of the list */
	maxOutputs = 0;				/* none yet */
	
	for (i1 = (numPTs -1); i1 >= 0; i1--) {
		if (nOutputs > maxOutputs) {		/* remember max # outputs needed */
			maxOutputs = nOutputs;
		}

		/* get inputs required for this fut as determined by outputs required from this fut */
		thisImask = 0;
		
		for (i2 = 0; i2 < FUT_NOCHAN; i2++) {
			if (thisOmask & FUT_BIT(i2)) {				/* is this output channel needed? */

				futChan = FCHANP(FFUTP(evalList[i1]->data)->chanHandle[i2]);	/* get pointer to chan */

				thisImask |= (KpUInt32_t)FUT_CHAN_IMASK(futChan);	/* include inputs required for this chan */
			}
		}

		ioMaskList [i1] = FUT_OUT(thisOmask) | FUT_IN(thisImask);	/* store the evaluation I/O mask */
		
		/* set up outputs for preceeding fut in the list */
		thisOmask = thisImask;				/* which are inputs to this fut */
		nOutputs = calcChans (thisOmask);	/* get number of output channels */
	}

	thisImask = calcChanMask (evalDef->dataTypeI, evalDef->nInputs, evalDef->input);	/* calculate input mask for the first fut */
	
	/* does the first fut have the required inputs? */
	if ((thisImask & FFUTP(evalList[0]->data)->iomask.in) != FFUTP(evalList[0]->data)->iomask.in) {
		PTErr = KCP_INVAL_EVAL;
		goto ErrOut;
	}

	/* Is temporary memory required? */
	/* If just one PT, then temporary data memory is not needed */
	/* if more than one PT (serial evaluation), then the number and size of the outputs */
	/* determines whether or not temporary memory is needed */
	
	PTErr = getDataBytes (evalDef->dataTypeI, &sizeInData);	/* get input data size */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut;
	}

	PTErr = getDataBytes (evalDef->dataTypeO, &sizeOutData);	/* get output data size */
	if (PTErr != KCP_SUCCESS) {
		goto ErrOut;
	}

	if (numPTs == 1) {
		(*tempMemNeeded) = 0;		/* not serial evaluation, no temporary data memory needed */
	}
	else {							/* serial evaluation */
									/* does this require temporary memory? */
		if ((maxOutputs > finalOutputs)
			|| (sizeInData < 2)
			|| (sizeOutData < 2)) {
			(*tempMemNeeded) = 1;				/* yes */
		}
		else {
			(*tempMemNeeded) = 0;				/* no */
		}
	}
	
ErrOut:
	return PTErr;
}


/* calculate channel usage mask */
KpUInt32_t
	calcChanMask (	KpInt32_t dataType,
					KpInt32_t nAddr,
					PTCompDef_p	addrList)
{
KpUInt32_t	chanMask;
KpInt32_t	i1;

	chanMask = 0;

	if (   (dataType == KCM_USHORT_555)
		|| (dataType == KCM_USHORT_565)
		|| (dataType == KCM_R10G10B10) ) {	/* single address, multiple channels */
		if (addrList[0].addr != NULL) {
			chanMask = FUT_BIT(0) | FUT_BIT(1) | FUT_BIT(2);
		}
	}
	else {									/* discrete channels */
		for (i1 = 0; i1 < nAddr; i1++) {
			if (addrList[i1].addr != NULL) {
				chanMask |= FUT_BIT(i1);
			}
		}
	}
	
	return chanMask;
}


/* calculate number of channels active in a channel usage mask */
KpInt32_t
	calcChans (	KpUInt32_t	mask)
{
KpInt32_t	nChans, i1;

	for (i1 = mask, nChans = 0; i1 != 0; i1 >>= 1) {	/* count total addresses needed */
		if ((i1 & 1) == 1) {
			nChans++;
		}
	}
	
	return nChans;		
}


/* returns the # of bytes per datum for a given data type */
static PTErr_t
	getDataBytes (	KpInt32_t	dataType,
					KpInt32_p	dataTypeSize)
{
	switch (dataType) {
	case KCM_USHORT_555:
	case KCM_USHORT_565:
	case KCM_R10G10B10:
		(*dataTypeSize) = 0;	/* less than 1 byte */
		break;

	case KCM_UBYTE:
		(*dataTypeSize) = 1;	/* 1 byte per datum */
		break;

	case KCM_USHORT_12:
	case KCM_USHORT:
		(*dataTypeSize) = 2;	/* 2 bytes per datum */
		break;

	default:
		return KCP_INVAL_DATA_TYPE;
	}

	return (KCP_SUCCESS);
}


#if defined(KCP_ACCEL)
PTErr_t
	PTEvalRdy (	opRefNum_t	opRefNum,
				KpInt32_p	progress)
{
	if (opRefNum || progress) {}

	return KCP_SUCCESS;
}


PTErr_t
	PTEvalCancel (	opRefNum_t opRefNum)
{
	if (opRefNum) {}

	return KCP_INVAL_OPREFNUM;
}
#endif

