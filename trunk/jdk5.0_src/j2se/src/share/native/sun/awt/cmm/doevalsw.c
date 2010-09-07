/*
 * @(#)doevalsw.c	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	@(#)doevalsw.c	2.93 99/03/04

	Contains:	Function evaluation with an image as input data.
				Handles single fut, serial futs, and multi-processors

	Written by:	The KCMS Team

	COPYRIGHT (c) 1992-2003 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include "kcms_sys.h"
#include "kcms_sys.h"
#include "fut.h"
#include "fut_util.h"
#include "kcptmgr.h"
#include "kcpcache.h"
#if defined (KCP_THREAD_MP)
#include "sithread.h"
#endif

#if defined (KCP_MACPPC_MP)
#include "mplib.h"
#endif

static KpUInt32_t format_analyze (imagePtr_p, KpInt32_p, KpUInt32_t);
static KpUInt32_t getEvalDataType (KpUInt32_t);

static formatFunc_t getFormatFuncI (KpUInt32_t, KpUInt32_t);
static formatFunc_t getFormatFuncO (KpUInt32_t, KpUInt32_t);

static PTErr_t getImageBounds (KpInt32_t imageLines, KpInt32_t lineStride,
								KpInt32_t imagePels, KpInt32_t pelStride,
						 		KpUInt8_p imageAddr, KpUInt8_p *startAddr, KpUInt8_p *finishAddr);

PTErr_t
	PTEvalSeq (		KpInt32_t		nFuts,
					PTTable_p*		evalList,
					KpUInt32_p		ioMaskList,
					PTEvalDTPB_p	evalDef,
					callBack_p		callBack)
{
PTErr_t			PTErr = KCP_SUCCESS;
KpUInt32_t		ifmt, ofmt;
KpInt32_t		i1, nInputs, nOutputs, numEvals, theTempPelStride, tmpps, tmpls;
PTTable_p		PTTableP;
PTImgAddr_t		tmpA;
evalControl_t	evalControl;

	/* Do not do anything if the input FuTs or evaluation parameters are NULL pointers */
	if ((nFuts == 0) || (nFuts > MAX_PT_CHAIN_SIZE) || (evalList == NULL) || (evalDef == NULL)) {
		return   KCP_INVAL_EVAL;
	}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	sprintf (string, "\nPTEvalSeq\n nPels %d, nLines %d, nInputs %d, dataTypeI %d, nOutputs %d, dataTypeO %d, callBack %x\n",
			evalDef->nPels, evalDef->nLines, evalDef->nInputs, evalDef->dataTypeI, evalDef->nOutputs, evalDef->dataTypeO, callBack);
	kcpDiagLog (string);
	sprintf (string, " nFuts %d; evalList[]->refNum", nFuts);
	for (i1 = 0; i1 < nFuts; i1++) {
		sprintf (str2, ", %x", evalList[i1]->refNum);
		strcat (string, str2);
	}
	strcat (string, "\n");
	kcpDiagLog (string); }
	#endif

	for (i1 = 0; i1 < nFuts; i1++) {
		evalControl.ioMaskList[i1] = ioMaskList[i1];	/* get list of i/o masks */
	}
	
	/* verify input and output data formats and set size of each component of the evaluation */
	evalControl.evalDataTypeI = getEvalDataType (evalDef->dataTypeI);
	evalControl.evalDataTypeO = getEvalDataType (evalDef->dataTypeO);

	if ((evalControl.evalDataTypeI == KCM_UNKNOWN) || (evalControl.evalDataTypeO == KCM_UNKNOWN)) {
		PTErr = KCP_INVAL_DATA_TYPE;
		goto GetOut;
	}

	/* Initialize the evaluation control structure for the main task */
	evalControl.callBack = callBack;
	evalControl.imageLines = evalDef->nLines;
	evalControl.imagePels = evalDef->nPels;
	evalControl.nFuts = nFuts;
	evalControl.evalList = evalList;
	
	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	strcpy (string, " input[]");
	for (i1 = 0; i1 < evalDef->nInputs; i1++) {
		sprintf (str2, "; %x, %d, %d", evalDef->input[i1].addr, evalDef->input[i1].pelStride, evalDef->input[i1].lineStride);
		strcat (string, str2);
	}
	strcat (string, "\n");
	kcpDiagLog (string); }
	#endif

	/* set up input address/stride arrays */
	for (i1 = 0, nInputs = 0; i1 < FUT_NICHAN; i1++) {
		if (evalDef->input[i1].addr != NULL) {
			nInputs++;							/* inputs in use */

			tmpA = evalDef->input[i1].addr;
			tmpps = evalDef->input[i1].pelStride;
			tmpls = evalDef->input[i1].lineStride;
		}
		else {
			tmpA = NULL;
			tmpps = 0;
			tmpls = 0;
		}

		evalControl.inputData[i1].pI = tmpA;
		evalControl.inPelStride[i1] = tmpps;
		evalControl.inLineStride[i1] = tmpls;
	}

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256], str2[256];
	KpInt32_t	i1;
	strcpy (string, " output[]");
	for (i1 = 0; i1 < evalDef->nOutputs; i1++) {
		sprintf (str2, "; %x, %d, %d", evalDef->output[i1].addr, evalDef->output[i1].pelStride, evalDef->output[i1].lineStride);
		strcat (string, str2);
	}
	strcat (string, "\n");
	kcpDiagLog (string); }
	#endif

	/* set up output address/stride arrays */
	for (i1 = 0, nOutputs = 0; i1 < FUT_NOCHAN; i1++) {
		if (evalDef->output[i1].addr != NULL) {
			nOutputs++;							/* outputs in use */

			tmpA = evalDef->output[i1].addr;
			tmpps = evalDef->output[i1].pelStride;
			tmpls = evalDef->output[i1].lineStride;
		}
		else {
			tmpA = NULL;
			tmpps = 0;
			tmpls = 0;
		}

		evalControl.outputData[i1].pI = tmpA;
		evalControl.outPelStride[i1] = tmpps;
		evalControl.outLineStride[i1] = tmpls;
	}

	/* initialize flags and set up optimized evaluation tables if needed */
   	numEvals = evalControl.imagePels * evalControl.imageLines;

	if ((nFuts == 1) &&		/* if just one fut (i.e. not serial) */
		  (((evalDef->dataTypeI == KCM_UBYTE) && (evalDef->dataTypeO == KCM_UBYTE))	/* and i/o precisions match */
		|| ((evalDef->dataTypeI == KCM_USHORT_12) && (evalDef->dataTypeO == KCM_USHORT))
		|| ((evalDef->dataTypeI == KCM_USHORT) && (evalDef->dataTypeO == KCM_USHORT_12))
		|| ((evalDef->dataTypeI == KCM_USHORT_12) && (evalDef->dataTypeO == KCM_USHORT_12))
		|| ((evalDef->dataTypeI == KCM_USHORT) && (evalDef->dataTypeO == KCM_USHORT))
		|| ((((evalDef->dataTypeI == KCM_UBYTE) && (evalDef->dataTypeO == KCM_USHORT_12)) ||	/* 8->12 */
		   ((evalDef->dataTypeI == KCM_USHORT_12) && (evalDef->dataTypeO == KCM_UBYTE))) &&	/* 12->8 */
		   (3 == nInputs) && (3 == nOutputs)))) {
		
		evalControl.compatibleDataType = 1;		/* data already in format compatible with evaluation functions */

		ifmt = format_analyze (evalControl.inputData, evalControl.inPelStride, evalControl.evalDataTypeI);   
		ofmt = format_analyze (evalControl.outputData, evalControl.outPelStride, evalControl.evalDataTypeO); 
	}
	else {
		evalControl.compatibleDataType = 0;		/* data is not in compatible format */

		/* set up temporary buffer pel strides */
		if (nFuts != 1) {	/* full precision intermediate results if serial evaluation */
			evalControl.evalDataTypeI = KCM_USHORT;				/* 16 bit intermediate data */
			theTempPelStride = 2;
		   	numEvals = FUT_EVAL_BUFFER_SIZE;	/* reduce # pels being evaluated */
		}
		else {		/* promote to larger of the two */
			if ((evalControl.evalDataTypeI == KCM_USHORT)
				|| (evalControl.evalDataTypeO == KCM_USHORT)) {
				evalControl.evalDataTypeI = KCM_USHORT;			/* 16 bit intermediate data */
				theTempPelStride = 2;
			}
			else {
				if ((evalControl.evalDataTypeI == KCM_USHORT_12)
					|| (evalControl.evalDataTypeO == KCM_USHORT_12)) {
					evalControl.evalDataTypeI = KCM_USHORT_12;	/* 12 bit intermediate data */
					theTempPelStride = 2;
				}
				else {
					evalControl.evalDataTypeI = KCM_UBYTE;		/* 8 bit intermediate data */
					theTempPelStride = 1;
				}
			}
		}

		for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
			evalControl.tempPelStride[i1] = theTempPelStride;
		}

		ifmt = FMT_GENERAL;	/* this means temp I/O buffers MUST be set up to planar arrays */
		ofmt = FMT_GENERAL; 
		evalControl.evalDataTypeO = evalControl.evalDataTypeI;	/* in and out are the same */

		/* get the input reformatting function */
		evalControl.formatFuncI = getFormatFuncI (evalDef->dataTypeI, evalControl.evalDataTypeI);	

		/* and the output reformatting function */
		evalControl.formatFuncO = getFormatFuncO (evalControl.evalDataTypeO, evalDef->dataTypeO);	
	}
	
	PTErr = getEvalFunc (numEvals, ifmt, ofmt, &evalControl);	/* get the evaluation function */
	if (PTErr != KCP_SUCCESS) {
		goto GetOut;
	}

	PTErr = initEvalTables (&evalControl);		/* set up tables */
	if (PTErr== KCP_SUCCESS) {
		PTErr = evalImageMP (&evalControl);		/* and evaluate! */
	}	

GetOut:
	for (i1 = 0; i1 < nFuts; i1++) {
		PTTableP = evalList[i1];
		fut_unlock_fut (PTTableP->dataP);	/* unlock all the futs */
		#if defined(KPMAC)
		PTTableP->dataP = NULL;				/* make sure unlocked memory is not used */
		#endif
		unlockEvalTables (PTTableP);		/* and the optimized tables */
	}		

	return PTErr;
}

	
PTErr_t
	evalImageMP (	evalControl_p	fullImageControl)
{
evalControl_t	evalControl[KCP_MAX_PROCESSORS];
evalControl_p	evalControlP;
PTErr_t			PTErr = KCP_SUCCESS, errnum1;
KpInt32_t		i1, i2, numWaitLoops, tasksStarted;
KpInt32_t		linesPerTask, totalLines, imageStride, linesRemainder, taskCount;
#if defined (KCP_MACPPC_MP)
KpUInt32_t		action, taskStatus;
processGlobals_p	pGP;
OSStatus		theStatus;
taskControl_p		taskListP;
#elif defined (KCP_THREAD_MP)
KpThread_t		thread[KCP_MAX_PROCESSORS];
#endif
initializedGlobals_p	iGP;
KpUInt8_p		inStartAddr, inFinishAddr, outStartAddr, outFinishAddr;


	iGP = getInitializedGlobals ();
	if (iGP == NULL) {
		PTErr = KCP_NO_PROCESS_GLOBAL_MEM;
		goto GetOut;
	}

	taskCount = MIN((KpInt32_t) iGP->numProcessors, KCP_MAX_PROCESSORS);

	if (taskCount > 1) {
		for (i1 = 0; i1 < FUT_NICHAN; i1++) {				/* check input image addresses */

			if (fullImageControl->inputData[i1].p8 == NULL ) continue;
		
			PTErr = getImageBounds (fullImageControl->imageLines, fullImageControl->inLineStride[i1],
							fullImageControl->imagePels, fullImageControl->inPelStride[i1],
							fullImageControl->inputData[i1].p8, &inStartAddr, &inFinishAddr);

			if (PTErr != KCP_SUCCESS) {
				goto GetOut;
			}
			PTErr = getImageBounds (fullImageControl->imageLines, fullImageControl->outLineStride[i1],
							fullImageControl->imagePels, fullImageControl->outPelStride[i1],
							fullImageControl->outputData[i1].p8, &outStartAddr, &outFinishAddr);

			if (PTErr != KCP_SUCCESS) {
				goto GetOut;
			}
			if ((fullImageControl->evalDataTypeI == fullImageControl->evalDataTypeO) &&
						(inStartAddr == outStartAddr) && (inFinishAddr == outFinishAddr)) continue;
					
			for (i2 = 0; i2 < FUT_NOCHAN; i2++) {			/* check output image addresses */
		
				if (fullImageControl->outputData[i2].p8 == NULL ) continue;
				PTErr = getImageBounds (fullImageControl->imageLines, fullImageControl->outLineStride[i2],
								fullImageControl->imagePels, fullImageControl->outPelStride[i2],
								fullImageControl->outputData[i2].p8, &outStartAddr, &outFinishAddr);

				if (PTErr != KCP_SUCCESS) {
					goto GetOut;
				}
				if (((outStartAddr >= inStartAddr) && (outStartAddr <= inFinishAddr)) ||
							 ((outFinishAddr >= inStartAddr) && (outFinishAddr <= inFinishAddr))) {
					taskCount = 1;		/* can only use 1 processor */
					break;
				}
			}
		}
	}
	
#if defined (KCP_MACPPC_MP)
	pGP = loadProcessGlobals ();
	if (pGP == NULL) {
		return KCP_NO_PROCESS_GLOBAL_MEM;
	}

	/* make a local copy so we do not have to keep globals locked */
	taskListP = pGP->taskListP;	

	if (pGP->isMPinitialized != 1) {
		taskCount = 1;			/* can only use 1 processor */
	}

	unloadProcessGlobals();
#endif

	/* #define KCP_MP_TEST 1	 */
#if defined (KCP_MP_TEST)
	taskCount = 4;					/* max for testing */
#endif

/* keep just one global cache of optimized lookup tables */
/* this single cache can be shared by all tasks, but only as read-only memory */
/* if the tasks need separate optimized luts - which should not happen - then */
/* they need to switch to the non-optimized luts mode to keep the memory demands reasonable */

	for (i1 = 0; i1 < KCP_MAX_PROCESSORS; i1++) {
		evalControl[i1] = *fullImageControl;			/* copy evaluation control into each task */
	}

	totalLines = evalControl[0].imageLines;

	if (totalLines < taskCount) {
		taskCount = totalLines; /* fixes timing in test suite */
	}

	linesPerTask = totalLines / taskCount;						/* calc lines each task will process */
	linesRemainder = totalLines - (linesPerTask * taskCount);	/* # of tasks that get an extra line */
	
	/* schedule the larger linecounts first */
	for (i1 = 0; i1 < linesRemainder; i1++) {
		evalControl[i1].imageLines = linesPerTask +1;	/* "remainder" tasks get an extra line */
	}

	for (i1 = linesRemainder; i1 < taskCount; i1++) {
		evalControl[i1].imageLines = linesPerTask;	/* "remainder" tasks get an extra line */
	}

	for (i1 = 1; i1 < taskCount; i1++) {
		evalControl[i1].callBack = NULL;	/* so that only main task uses progress call-back */

		/* calc start address for this task */
		linesPerTask = evalControl[i1 -1].imageLines;	/* lines this task will process */

		for (i2 = 0; i2 < FUT_NICHAN; i2++) {			/* input image addresses for this task */
			imageStride = evalControl[0].inLineStride[i2] * linesPerTask;	/* lines done by previous task */
			evalControl[i1].inputData[i2].p8 = evalControl[i1 -1].inputData[i2].p8 + imageStride;
		}

		for (i2 = 0; i2 < FUT_NOCHAN; i2++) {			/* output image addresses for this task */
			imageStride = evalControl[0].outLineStride[i2] * linesPerTask;	/* lines done by previous task */
			evalControl[i1].outputData[i2].p8 = evalControl[i1 -1].outputData[i2].p8 + imageStride;
		}
	}


	/* Initialize the progress, calling it no more than 1% of the time. */
	numWaitLoops = (totalLines + 99) / 100;
	initProgress (numWaitLoops, evalControl[0].callBack);

	PTErr = doProgress (evalControl[0].callBack, 0);	/* always send 0 at start */
	if (PTErr != KCP_SUCCESS) {
		goto GetOut;
	}

	/* send evaluation command to each task */
	/* need to wait for each started task to keep queues synchronized */
	/* otherwise, the completion queue for a task started this time */
	/* might be erroneously seen as the completion for a task started next time */
	tasksStarted = 0;

	for (i1 = 1; i1 < taskCount; i1++) {
		evalControlP = &evalControl[i1];
		
#if !defined (KCP_MP_TEST)
#if defined (KCP_MACPPC_MP)
		theStatus = MPNotifyQueue (taskListP[i1].fromMain, 
			(void *)kEvaluate, (void *)evalControlP, 0);
		if (theStatus != noErr) {
			/* for testing */
			/* DebugStr ("\pMPNotifyQueue error");	*/
			PTErr = KCP_NOT_COMPLETE;
			break;
		}
#elif defined (KCP_THREAD_MP)
		thread[i1-1] = KpThreadCreate ((KpThrStartFunc)evalImage, 
			evalControlP, NULL, 0, NULL);
#endif
#endif
		tasksStarted++;		/* count tasks started */
	}

	/* evaluate the main task image */
	errnum1 = evalImage (&evalControl[0]);
	if ((PTErr == KCP_SUCCESS) && (errnum1 != KCP_SUCCESS)) {
		PTErr = errnum1;		/* save the first error */
	}

	/* wait for all tasks to finish */
#if defined (KCP_THREAD_MP)
	for (i1 = 0; i1 < taskCount-1; i1++) {
		KpThreadWait (&thread[i1], 1, THREAD_WAIT_ONE, 
			THREAD_TIMEOUT_INFINITE, NULL);
		KpThreadDestroy (thread[i1]);
	}
#else	
	for (i1 = 1; i1 < tasksStarted +1; i1++) {
#if defined (KCP_MACPPC_MP)
#if !defined (KCP_MP_TEST)
		theStatus = MPWaitOnQueue (taskListP[i1].toMain, 
			(void **)&action, (void **)&taskStatus, NULL, 
			kDurationForever);
		if (theStatus == noErr) {
			/* copy to avoid enum size problems */
			errnum1 = (PTErr_t)taskStatus;
		}
		else {
			/* for testing */
			/* DebugStr ("\pMPWaitOnQueue error");	*/
			errnum1 = KCP_NOT_COMPLETE;
		}
#else
		theStatus = noErr;
		action = 0;
		taskStatus = KCP_SUCCESS;
		evalControlP = &evalControl[i1];
		/* evaluate the sub-image */
		errnum1 = evalImage (evalControlP);	
#endif
#endif
		if ((PTErr == KCP_SUCCESS) && (errnum1 != KCP_SUCCESS)) {
			/* for testing */
			/* DebugStr ("\pError in completion loop"); */

			/* save the first error */
			PTErr = errnum1;		
		}
	}
#endif
	

GetOut:
	if (PTErr == KCP_SUCCESS) {
		for (i1 = 1; i1 < taskCount; i1++) {
			if (evalControl[i1].PTErr != KCP_SUCCESS) {
				PTErr = evalControl[i1].PTErr;			 /* did any thread fail? */
				break;
			}
		}
	}

	if (PTErr == KCP_SUCCESS) {
		PTErr = doProgress (evalControl[0].callBack, 100);	/* always send 100 at end */
	}

	return PTErr;
}


/**********************************************************************/

#if defined (KCP_MACPPC_MP)

OSStatus
	evalTaskMac (	void*	taskParameter)
{
PTErr_t			PTErr;
KpUInt32_t		finished, action;
taskControl_p	taskControl;
evalControl_p	evalControlP;
OSStatus		theStatus;

	taskControl = (taskControl_p)taskParameter;

	finished = 0;

	while (finished == 0) {
		theStatus = MPWaitOnQueue(taskControl->fromMain, (void **)&action, (void **)&evalControlP, NULL, kDurationForever);
		if (theStatus != noErr) {
			action = kErrMP;		/* MP error */
		}

		switch	(action) {
			case kEvaluate:
				PTErr = evalImage (evalControlP);		/* evaluate the image */

				break;
				
			case kTerminate:
				PTErr = KCP_SUCCESS;
				finished = 1;
				break;
				
			case kErrMP:
				PTErr = KCP_NOT_COMPLETE;
				finished = 1;
				break;
				
			default:
				PTErr = KCP_FAILURE;
				break;
			}

		theStatus = MPNotifyQueue(taskControl->toMain, (void *)kComplete, (void *)PTErr, NULL);
		if (theStatus != noErr) {
			PTErr = KCP_NOT_COMPLETE;
			finished = 1;
		}
	}

	return	(theStatus);
}

#endif


/**********************************************************************/

PTErr_t
	evalImage (	evalControl_p	eC)
{
PTErr_t			PTErr = KCP_SUCCESS;
PTTable_p		PTTableP;
KpInt32_t		i1, np, linecnt, numPels, numFuts, linecnt100times;

imagePtr_t		iLineData[FUT_NICHAN], oLineData[FUT_NOCHAN];
imagePtr_t		itempData[FUT_NCHAN], otempData[FUT_NCHAN];
imagePtr_t		tempData0[FUT_NCHAN], tempData1[FUT_NCHAN], tempData2[FUT_NCHAN];
imagePtr_p		otempData0, otempData1, tempDataP;
KpUInt16_t		tempData [2][FUT_NCHAN][FUT_EVAL_BUFFER_SIZE];

	linecnt100times = eC->imageLines * 100;

/* Loop over output picture rows */
	for (linecnt = 0; linecnt < linecnt100times; linecnt+=100) {

		PTErr = doProgress (eC->callBack, linecnt / eC->imageLines);
		if (PTErr != KCP_SUCCESS) {
			goto GetOut;
		}

		/* evaluate a line of data */
		if (eC->compatibleDataType == 1) {
			((evalTh1Proc_t)(eC->evalFunc))
				(eC->inputData, eC->inPelStride, eC->evalDataTypeI,
				eC->outputData, eC->outPelStride, eC->evalDataTypeO,
				eC->imagePels, eC->evalList[0]);
		}
		else {	/* some special format data or a serial evaluation */

			/* initialize data pointers for this line */
			for (i1 = 0; i1 < FUT_NICHAN; i1++) {
				iLineData[i1] = eC->inputData[i1];
			}

			for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
				oLineData[i1] = eC->outputData[i1];
			}

			for (i1 = 0; i1 < FUT_NCHAN; i1++) {
				tempData0 [i1].p16 = tempData [0][i1];
				tempData1 [i1].p16 = tempData [1][i1];
			}

			/* do each line in small groups */
			np = FUT_EVAL_BUFFER_SIZE;
			for ( numPels = eC->imagePels; numPels > 0; numPels -= FUT_EVAL_BUFFER_SIZE ) {
				KpUInt32_t	imask;

				if ( numPels < FUT_EVAL_BUFFER_SIZE ) {     /* last group will be smaller */
					np = numPels;
				}

				otempData0 = tempData0;	/* initialize temp buffer address lists */ 
				otempData1 = tempData1;

				imask = FUT_IMASK(eC->ioMaskList [0]);

				for (i1 = 0; i1 < FUT_NICHAN; i1++) {		/* set up reformatting destination pointers */
					if ((FUT_BIT(i1) & imask) != 0) {
						itempData [i1] = otempData0 [i1];	/* 1st input is in 1st temp buffer */
					}
					else {
						itempData[i1].p8 = NULL;
					}
					tempData2[i1] = itempData[i1];			/* for the input format function, which changes dest addresses */
				}

				((formatFunc_t)(eC->formatFuncI)) (np, iLineData, eC->inPelStride, tempData2);	/* format the input data */

				for (numFuts = 0; numFuts < eC->nFuts; numFuts++) {		/* evaluate each fut in the list */
					KpUInt32_t	omask;

					PTTableP = eC->evalList[numFuts];			/* get the next fut */

					/* set up temp output addresses */
					omask = FUT_OMASK(eC->ioMaskList [numFuts]);
					for (i1 = 0; i1 < FUT_NCHAN; i1++) {
						if ((FUT_BIT(i1) & omask) != 0) {
							otempData [i1] = otempData1 [i1];
						}
						else {
							otempData [i1].p8 = NULL;
						}
					}

					((evalTh1Proc_t)(eC->evalFunc)) (itempData, eC->tempPelStride, eC->evalDataTypeI,
													otempData, eC->tempPelStride, eC->evalDataTypeO,
													np, PTTableP);

					/* copy output addresses to input */
					for (i1 = 0; i1 < FUT_NCHAN; i1++) {
						itempData [i1] = otempData [i1];
					}

					tempDataP = otempData0;			/* swap buffers */
					otempData0 = otempData1;
					otempData1 = tempDataP;
				}

				((formatFunc_t)(eC->formatFuncO)) (np, otempData, eC->outPelStride, oLineData);	/* format the output data */
			}
		}

		/* move inputs to the next line */
		for (i1 = 0; i1 < FUT_NICHAN; i1++) {
			eC->inputData[i1].p8 += eC->inLineStride[i1];
		}

		/* move outputs to the next line */
		for (i1 = 0; i1 < FUT_NOCHAN; i1++) {
			eC->outputData[i1].p8 += eC->outLineStride[i1];
		}
	}

GetOut:
	eC->PTErr = PTErr;	/* stick it here for MP threads */
	return	(PTErr);
}


/**********************************************************************/

static KpUInt32_t
	getEvalDataType (KpUInt32_t dataType)
{
KpUInt32_t	evalDataType;

	switch (dataType) {
	case KCM_USHORT_555:
	case KCM_USHORT_565:
	case KCM_UBYTE:
		evalDataType = KCM_UBYTE;
		break;
		
	case KCM_R10G10B10:
	case KCM_USHORT_12:
		evalDataType = KCM_USHORT_12;
		break;
		
	case KCM_USHORT:
		evalDataType = KCM_USHORT;
		break;

	default:
		evalDataType = KCM_UNKNOWN;
	}
	
	return evalDataType;
}


/**********************************************************************/

static formatFunc_t
	getFormatFuncI (	KpUInt32_t		dataTypeI,
						KpUInt32_t		dataTypeO)
{
formatFunc_t	formatFuncI = NULL;

	switch (dataTypeO) {
	case KCM_UBYTE:
		switch (dataTypeI) {
		case KCM_UBYTE:
			formatFuncI = pass8in;
			break;
			
		case KCM_USHORT_555:
			formatFuncI = format555to8;
			break;
			
		case KCM_USHORT_565:
			formatFuncI = format565to8;
			break;
			
		case KCM_R10G10B10:		/* would be 12->8, should not happen */
		case KCM_USHORT_12:		/* would be 12->8, should not happen */
		case KCM_USHORT:		/* would be 16->8, should not happen */
		default:
			break;
		}

		break;
		
	case KCM_USHORT_12:
		switch (dataTypeI) {
		case KCM_UBYTE:
			formatFuncI = format8to12;
			break;
			
		case KCM_USHORT_12:
			formatFuncI = pass16in;
			break;
			
		case KCM_USHORT:
			formatFuncI = format16to12;
			break;
			
		case KCM_USHORT_555:
			formatFuncI = format555to12;
			break;
			
		case KCM_USHORT_565:
			formatFuncI = format565to12;
			break;
			
		case KCM_R10G10B10:
			formatFuncI = format10to12;
			break;
			
		default:
			break;
		}
		
		break;
		
	case KCM_USHORT:
		switch (dataTypeI) {
		case KCM_UBYTE:
			formatFuncI = format8to16;
			break;
			
		case KCM_USHORT_12:
			formatFuncI = format12to16;
			break;
			
		case KCM_USHORT:
			formatFuncI = pass16in;
			break;
			
		case KCM_USHORT_555:
			formatFuncI = format555to16;
			break;
			
		case KCM_USHORT_565:
			formatFuncI = format565to16;
			break;
			
		case KCM_R10G10B10:
			formatFuncI = format10to16;
			break;
			
		default:
			break;
		}
		
		break;

	default:
		break;
	}
	
	return (formatFuncI);
}


/**********************************************************************/

static formatFunc_t
	getFormatFuncO (	KpUInt32_t		dataTypeI,
						KpUInt32_t		dataTypeO)
{
formatFunc_t	formatFuncO = NULL;

	switch (dataTypeI) {
	case KCM_UBYTE:
		switch (dataTypeO) {
		case KCM_UBYTE:			/* compatible */
			formatFuncO = pass8out;
			break;
			
		case KCM_USHORT_555:
			formatFuncO = format8to555;
			break;
			
		case KCM_USHORT_565:
			formatFuncO = format8to565;
			break;
			
		case KCM_R10G10B10:		/* would be 8->12, should not happen */
		case KCM_USHORT_12:		/* would be 8->12, should not happen */
		case KCM_USHORT:		/* would be 8->16, should not happen */
		default:
			break;
		}

		break;
		
	case KCM_USHORT_12:
		switch (dataTypeO) {
		case KCM_UBYTE:
			formatFuncO = format12to8;
			break;
			
		case KCM_USHORT_12:
			formatFuncO = pass16out;
			break;
			
		case KCM_USHORT:
			formatFuncO = format12to16;
			break;
			
		case KCM_USHORT_555:
			formatFuncO = format12to555;
			break;
			
		case KCM_USHORT_565:
			formatFuncO = format12to565;
			break;
			
		case KCM_R10G10B10:
			formatFuncO = format12to10;
			break;
			
		default:
			break;
		}

		break;
		
	case KCM_USHORT:
		switch (dataTypeO) {
		case KCM_UBYTE:
			formatFuncO = format16to8;
			break;
			
		case KCM_USHORT_12:
			formatFuncO = format16to12;
			break;
			
		case KCM_USHORT:
			formatFuncO = pass16out;
			break;
			
		case KCM_USHORT_555:
			formatFuncO = format16to555;
			break;
			
		case KCM_USHORT_565:
			formatFuncO = format16to565;
			break;
			
		case KCM_R10G10B10:
			formatFuncO = format16to10;
			break;
			
		default:
			break;
		}
		
		break;

	default:
		break;
	}
	
	return (formatFuncO);
}


/**********************************************************************/

/* format_analyze determines the format of the input or output image */
static KpUInt32_t
	format_analyze (imagePtr_p		p,
					KpInt32_p		s,
					KpUInt32_t		dataType)
{
KpUInt32_t	fmt = FMT_GENERAL;	/* Default foramt in case none of the below cases fit */
KpInt32_t	t, pStride, cStride;
#if defined(KPMAC)	/* must allow for word R/W frame buffers */
KpUInt8_p	base;
#endif

	/* determine if all the strides are the same (or zero) and if so,
	 * what that stride is.  If any non-zero stride differs from any
	 * other non-zero stride, return with FMT_GENERAL.  Otherwise, we
	 * continue on to check for pixel interleaved formats.
	 *
	 * NOTE: the following "if" statement, although cryptic, really works!!
	 *       (And it does so with a maximum of 9 "tests").
	 */
	pStride = s[0];
	if ( ((t=s[1]) != 0  &&  t != (pStride = pStride ? pStride : t)) ||
	     ((t=s[2]) != 0  &&  t != (pStride = pStride ? pStride : t)) ||
	     ((t=s[3]) != 0  &&  t != (pStride = pStride ? pStride : t)) ) {

		return (FMT_GENERAL);
	}

	/* If we get to here, all non-zero strides are the same.
	 * Now, check for various special case formats. */
	switch (pStride) {
		case 3:
			if ((p[0].p8+1 == p[1].p8) && (p[1].p8+1 == p[2].p8) && (p[3].p8 == NULL)) {
				fmt = FMT_BIGENDIAN24;
			}
			else {
				if ((p[0].p8-1 == p[1].p8) && (p[1].p8-1 == p[2].p8) && (p[3].p8 == NULL)) {
					fmt = FMT_LITTLEENDIAN24;
				}
			}
			break;

		case 4:     /* check for 32-bit formats */

#if defined(KPMAC)	/* must allow for word R/W frame buffers */
			base = (KpUInt8_p) ((KpInt32_t) (p[0].p8) & 0xfffffffc);
			if ((base+1 == p[0].p8) && (p[0].p8+1 == p[1].p8) && (p[1].p8+1 == p[2].p8) && (p[3].p8 == NULL)) {
				fmt = FMT_QD;
			}
#endif
			if ((p[0].p8+1 == p[1].p8) && (p[1].p8+1 == p[2].p8) && (p[2].p8+1 == p[3].p8)) {
				fmt = FMT_BIGENDIAN32;
			}
			else {
				if ((p[0].p8-1 == p[1].p8) && (p[1].p8-1 == p[2].p8) && (p[2].p8-1 == p[3].p8)) {
					fmt = FMT_LITTLEENDIAN32;
				}
			}
			break;

		default:
			/* cStride is the component stride if they are all the same, otherwise it is 0 */
			cStride = p[1].p8 - p[0].p8;
			if (cStride != (p[2].p8 - p[1].p8)) {
				cStride = 0;
			}
			else {
				if ((p[3].p8 != NULL) && (cStride != (p[3].p8 - p[2].p8))) {
					cStride = 0;
				}
			}

			if (cStride != 0) {
				fmt = FMT_EQSTRIDES;	/* pixel strides are the same and component strides are the same */
			}
			else {
				fmt = FMT_GENERAL;	    /* use general format */
			}
			break;
	}

	return (fmt);

}


static PTErr_t
	getImageBounds (KpInt32_t imageLines, KpInt32_t lineStride, KpInt32_t imagePels, KpInt32_t pelStride,
						 KpUInt8_p imageAddr, KpUInt8_p *startAddr, KpUInt8_p *finishAddr)
{
KpUInt8_p	tempAddr;

	*startAddr = *finishAddr = imageAddr;

	if (imageLines > 1) {
		tempAddr = *startAddr + (lineStride * imageLines);
	} else if (imagePels > 1) {
		tempAddr = *startAddr + (pelStride * imagePels);
	}
	if (tempAddr < *startAddr) {
		*startAddr = tempAddr;
	} else if (tempAddr > *finishAddr) {
		*finishAddr = tempAddr;
	}
	return (KCP_SUCCESS);
}
	
