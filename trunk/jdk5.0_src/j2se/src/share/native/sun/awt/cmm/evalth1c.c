/*
 * @(#)evalth1c.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	evalth1c.c

	Contains:	tetrahedral interpolation with optimized tables

	Author:		George Pawle

	COPYRIGHT (c) 1996-2003 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "fut_util.h" 
#include "attrib.h"
#include "kcpcache.h"

static evalTh1Proc_t getTh1EvalFuncOpt (evalControl_p, KpUInt32_t, KpUInt32_t, KpInt32_p);
static void initGridInfo (KpInt32_t, KpInt32_t, PTTable_p, fut_gtbl_p);
static void th1MFtbl2InLut (mf2_tbldat_p, KpInt32_t, KpInt32_t, KpInt32_t, KpInt32_t, KpUInt32_t, KpGenericPtr_t);
static void freeEtMem (etMem_p);
static void nullEtMem (etMem_p);
static void lockEtMem (etMem_p);
static void unlockEtMem (etMem_p);
static void allocEtMem (etMem_p, KpInt32_t);
static KpInt32_t hasEtMem (etMem_p);
#if defined KCP_SINGLE_EVAL_CACHE
static PTErr_t	getEvalStatePT (PTRefNum_p);
static PTErr_t	putEvalStatePT (PTRefNum_t);
#endif
static void checkForAccelerators (evalControl_p);

/******************************************************************************/
/* get the evaluation function to use */
PTErr_t
	getEvalFunc (	KpInt32_t		nEvals,
					KpUInt32_t		ifmt,
					KpUInt32_t		ofmt,
					evalControl_p	evalControlP)
{
KpInt32_t	numOutputs, totalEvals;

	evalControlP->optimizedEval = 0;			/* assume that optimized tables can not be used */

	checkForAccelerators (evalControlP);		/* which accelerators are present? */

	if (evalControlP->nFuts == 1) {
		evalControlP->evalFunc = getTh1EvalFuncOpt (evalControlP, ifmt, ofmt, &numOutputs);		/* try for optimized function */

		if (evalControlP->evalFunc != NULL) {
			totalEvals = numOutputs * nEvals;
				
			if ( !		/* if none of the following apply... */
				(((((evalControlP->evalDataTypeI == KCM_USHORT) || (evalControlP->evalDataTypeO == KCM_USHORT)) && (totalEvals < TH1_MIN_16BIT_EVALS))	/* verify sufficient # of evaluations */
				|| (((evalControlP->evalDataTypeI != KCM_USHORT) && (evalControlP->evalDataTypeO != KCM_USHORT)) && (totalEvals < TH1_MIN_EVALS)))
				&& (ifmt != FMT_QD) && (ofmt != FMT_QD))) {		/* still need to use iQDoQD for QD */

				evalControlP->optimizedEval = 1;	/* use optimized method */
			}
		}
	}

	if (evalControlP->optimizedEval == 0) {
		evalControlP->evalFunc = evalTh1gen;
	}	

	return KCP_SUCCESS;
}


/************************************************/
/* Check for hardware accelerators.
*/

static void
	checkForAccelerators (evalControl_p evalControlP)
{
#if defined KCP_PENTIUM
KpInt32_t	processorType;

	evalControlP->hasSSE2 = 0;
	evalControlP->hasSSE = 0;

	processorType = DetectProcessor ();

	if (processorType == SSE2_PRESENT) {
		evalControlP->hasSSE2 = 1;
	}
	else if (processorType == SSE_PRESENT) {
		evalControlP->hasSSE = 1;
	}
#endif
}


/************************************************/
/* See if this evaluation has been optimized.
   Returns function address if available.
*/

static evalTh1Proc_t
	getTh1EvalFuncOpt (	evalControl_p	evalControlP,
						KpUInt32_t		ifmt,
						KpUInt32_t		ofmt,
						KpInt32_p		numOutputsP)
{
evalTh1Proc_t	func;
KpUInt32_t 	iomask, imask, omask;
KpInt32_t 	i2, nIn, o, nOut;
fut_chan_p	futChan;
KpHandle_t	futH;

/*	return NULL;	*/ /* return null to force non-optimized evaluation */

	iomask = evalControlP->ioMaskList [0];
	imask = (KpInt32_t)FUT_IMASK(iomask);
	omask = (KpInt32_t)FUT_OMASK(iomask);

	futH = (KpHandle_t) evalControlP->evalList[0]->data;	/* check the fut */

	/* find number of output channels */
	for (nOut = 0, o = 0; o < FUT_NOCHAN; o++) {
		if (omask & FUT_BIT(o)) {
			for (nIn = 0, i2 = 0; i2 < FUT_NICHAN; i2++) {	/* make sure input tables are shared */
				if (imask & FUT_BIT(i2)) {
				
					futChan = FCHANP(FFUTP(futH)->chanHandle[o]);	/* get pointer to chan */

					if (futChan->itblHandle[i2] != FFUTP(futH)->itblHandle[i2]) {
						return NULL;		/* Input tables not shared */
					}
					nIn++;
				}
			}
			nOut++;
		}
	}

	*numOutputsP = nOut;

	evalControlP->iLutFormat = ET_ITBL_FMT1;		/* assume integer formats */
	evalControlP->gLutFormat = ET_GTBL_FMT1;
	evalControlP->iLutEntrySize = sizeof(evalILuti_t);
	evalControlP->gLutEntrySize = sizeof(mf2_tbldat_t);

	switch (evalControlP->evalDataTypeI) {
	case KCM_UBYTE:
		switch (nIn) {	
		case 3:
			switch (nOut) {
				case 1:
					func = evalTh1i3o1d8;
					break;

				case 2:
					func = evalTh1i3o2d8;
					break;

				case 3:
					func = evalTh1i3o3d8;
				
		#if defined (KPMAC)
					if (ifmt == FMT_QD) {
						if (ofmt == FMT_QD) {
							func = evalTh1iQDoQD;
						}
						else {
							func = evalTh1iQDo3;
						}
					}
					else {
						if (ofmt == FMT_QD) {
							func = evalTh1i3oQD;
						}
						else {
		#endif
							/* look for 3-channel 8-bit to 3-channel 12-bit */
							if (KCM_USHORT_12 == evalControlP->evalDataTypeO) {
								func = evalTh1i3o3d8to16;
							}
							else {/* otherwise, perform nominal checks */
								if ((ifmt == FMT_BIGENDIAN24) && (ofmt == FMT_BIGENDIAN24)) {
									func = evalTh1iB24oB24;
								}
								else {
									if ((ifmt == FMT_LITTLEENDIAN24) && (ofmt == FMT_LITTLEENDIAN24)) {
										func = evalTh1iL24oL24;
									}
								}
							}
		#if defined (KPMAC)
						}
					}
		#endif
					break;

				case 4:
					func = evalTh1i3o4d8;
					break;	  

				case 5:
					func = evalTh1i3o5d8;
					break;

				case 6:
					func = evalTh1i3o6d8;
					break;

				case 7:
					func = evalTh1i3o7d8;
					break;

				case 8:
					func = evalTh1i3o8d8;
					break;

				default:
					func = NULL;
					break;
			}
			
			break;
			
		case 4:
			switch (nOut) {
				case 1:
					func = evalTh1i4o1d8;
					break;

				case 2:
					func = evalTh1i4o2d8;
					break;

				case 3:
					func = evalTh1i4o3d8;

		#if defined (KPMAC)
					if (ofmt == FMT_QD) {
						func = evalTh1i4o3QD;
					}
		#endif
					break;

				case 4:
					func = evalTh1i4o4d8;

					if ((ifmt == FMT_BIGENDIAN32) && (ofmt == FMT_BIGENDIAN32)) {
						func = evalTh1iB32oB32;
					}
					else {
						if ((ifmt == FMT_LITTLEENDIAN32) && (ofmt == FMT_LITTLEENDIAN32)) {
							func = evalTh1iL32oL32;
						}
					}
					
					break;	  

				default:
					func = NULL;
					break;
			}
			break;

			default:
				func = NULL;
				break;
		}
		break;

	case KCM_USHORT_12:
	case KCM_USHORT:
		switch (nIn) {
		case 3:
			switch (nOut) {
				case 1:
					func = evalTh1i3o1d16;
					break;

				case 2:
					func = evalTh1i3o2d16;
					break;

				case 3:
					/* look for 3-channel 12/16-bit to 3-channel 8-bit */
					if (evalControlP->evalDataTypeO == KCM_UBYTE) {
						func = evalTh1i3o3d16to8;
					} else {
						func = evalTh1i3o3d16;
				#if defined (KCP_PENTIUM)
						if (((ifmt == FMT_EQSTRIDES) && (ofmt == FMT_EQSTRIDES)) &&
							(evalControlP->evalDataTypeI == KCM_USHORT_12) &&
							(evalControlP->evalDataTypeO == KCM_USHORT_12)) {
							if (evalControlP->hasSSE2 == 1) {
								func = evalTh1i3o3D12ISSE2;
								evalControlP->iLutFormat = ET_ITBL_FMT2;	/* SSE2 formats */
								evalControlP->iLutEntrySize = sizeof(evalILutSSE2_t);
								evalControlP->gLutFormat = ET_GTBL_FMT2;
								evalControlP->gLutEntrySize = sizeof(evalGLutSSE2_t);
							}
							else if (evalControlP->hasSSE == 1) {
								func = evalTh1i3o3D12ISSE;
								evalControlP->iLutFormat = ET_ITBL_FMT3;	/* SSE formats */
								evalControlP->iLutEntrySize = sizeof(evalILutSSE_t);
								evalControlP->gLutFormat = ET_GTBL_FMT3;
								evalControlP->gLutEntrySize = sizeof(evalGLutSSE_t);
							}
						}
				#endif
					}
					break;

				case 4:
					func = evalTh1i3o4d16;
					break;	  

				case 5:
					func = evalTh1i3o5d16;
					break;

				case 6:
					func = evalTh1i3o6d16;
					break;

				case 7:
					func = evalTh1i3o7d16;
					break;

				case 8:
					func = evalTh1i3o8d16;
					break;

				default:
					func = NULL;
					break;
			}
			break;
			
		case 4:
			switch (nOut) {
				case 1:
					func = evalTh1i4o1d16;
					break;

				case 2:
					func = evalTh1i4o2d16;
					break;

				case 3:
					func = evalTh1i4o3d16;
					break;

				case 4:
					func = evalTh1i4o4d16;
					break;	  

				default:
					func = NULL;
					break;
			}
			break;

		default:
			func = NULL;
			break;
		}
		break;

	default:
		func = NULL;
		break;
	}

	return (func);
}

 
/******************************************************************************/
/* initialize the evaluation tables */
PTErr_t
	initEvalTables (	evalControl_p	evalControlP)
{
KpInt32_t		theSizef, i1, i2, numInputs, nOutputChans, endBits, fracBits;
PTTable_p		PTTableP;
fut_p			fut;
fut_itbl_p		theITbl, iTblsP [FUT_NICHAN];
fut_chan_p		thisChan;
fut_gtbl_p		firstGTbl, aGTbl;
fut_otbl_p		oTblsP [FUT_NOCHAN];
mf2_tbldat_p	iTblDat;
mf2_tbldat_p	gTblsP [FUT_NOCHAN], interleavedGridP;
PTErr_t			PTErr = KCP_SUCCESS;
KpInt32_t		j, inputTableEntries, gridTblEntries, outputTableEntries, outputTableEntrySize, destTableMaxValue;
KpInt32_t		totalInputLutBytes, totalGridTableBytes, outputLutBytes, totalOutputLutBytes;
evalILuti_p		iLuti;
mf2_tbldat_t	tmp, identityTable[] = {0,0xffff};
etMem_p			etILutsP, etGLutsP, etOLutsP;
#if defined KCP_SINGLE_EVAL_CACHE
PTRefNum_t		evalStatePT;
#endif
float			gTableFactor;


	KpEnterCriticalSection (&PTCacheCritFlag);

	PTTableP = evalControlP->evalList[0];

	/* Input Tables controls */
	switch (evalControlP->evalDataTypeI) {
	case KCM_UBYTE:		
		inputTableEntries = 1 << 8;
		etILutsP = &PTTableP->etLuts[ET_I8];
		break;
		
	case KCM_USHORT_12:		
		inputTableEntries = 1 << 12;
		etILutsP = &PTTableP->etLuts[ET_I12F1];
#if defined KCP_PENTIUM
		if (evalControlP->iLutFormat == ET_ITBL_FMT2) {
			etILutsP = &PTTableP->etLuts[ET_I12F2];
		}
		else if (evalControlP->iLutFormat == ET_ITBL_FMT3) {
			etILutsP = &PTTableP->etLuts[ET_I12F3];
		}

#endif
		break;
		
	case KCM_USHORT:		
		inputTableEntries = 1 << 16;
		etILutsP = &PTTableP->etLuts[ET_I16];
		break;
		
	default:
		goto ErrOut1;
	}

	/* set up grid and output table controls */
	/* based on the macro KCP_CONVERT_DOWN(data, startBits, endBits) */
	switch (evalControlP->evalDataTypeO) {
	case KCM_UBYTE:		
		endBits = 12;					/* use 12 bit grid table data */
		fracBits = EVAL_D8_FRACBITS;	/* and fraction for 8-bit data */
		etGLutsP = &PTTableP->etLuts[ET_G12F1];
		etOLutsP = &PTTableP->etLuts[ET_O8];
		outputTableEntries = EVAL_OLUT_ENTRIESD8;
		outputTableEntrySize = sizeof(KpUInt8_t);
		destTableMaxValue = 0xff;
		break;
		
	case KCM_USHORT_12:		
		endBits = 16;					/* use 16 bit grid table data */
		fracBits = EVAL_D16_FRACBITS;	/* and fraction for 16-bit data */
#if defined KCP_PENTIUM
		if (evalControlP->iLutFormat == ET_ITBL_FMT2) {
			fracBits = EVAL_FMT2_FRACBITS;	/* and fraction for format 2 data */
		}
#endif
		etGLutsP = &PTTableP->etLuts[ET_G16];
		etOLutsP = &PTTableP->etLuts[ET_O12];
		outputTableEntries = EVAL_OLUT_ENTRIESD16;
#if defined KCP_PENTIUM
		if (evalControlP->gLutFormat == ET_GTBL_FMT2) {
			etGLutsP = &PTTableP->etLuts[ET_GF2];
			outputTableEntries = EVAL_OLUT_ENTRIES_FMT2;
			endBits = EVAL_OLUT_BITS_FMT2;		/* match grid table bits to size of output table */
		}
		else if (evalControlP->gLutFormat == ET_GTBL_FMT3) {
			etGLutsP = &PTTableP->etLuts[ET_GF3];
			outputTableEntries = EVAL_OLUT_ENTRIES_FMT3;
			gTableFactor = (outputTableEntries -1) / (float)MF2_TBL_MAXVAL;
		}
#endif
		outputTableEntrySize = sizeof(KpUInt16_t);
		destTableMaxValue = 0xfff;
		break;
		
	case KCM_USHORT:		
		endBits = 16;					/* use 16 bit grid table data */
		fracBits = EVAL_D16_FRACBITS;	/* and fraction for 16-bit data */
		etGLutsP = &PTTableP->etLuts[ET_G16];
		etOLutsP = &PTTableP->etLuts[ET_O16];
		outputTableEntries = EVAL_OLUT_ENTRIESD16;
		outputTableEntrySize = sizeof(KpUInt16_t);
		destTableMaxValue = 0xffff;
		break;
		
	default:
		goto ErrOut1;
	}

	/* if not optimized or any table is not ready, must lock the futs */
	if ((evalControlP->optimizedEval == 0) ||
		(hasEtMem (etILutsP) != 1) || (hasEtMem (etGLutsP) != 1) || (hasEtMem (etOLutsP) != 1)) {

		/* lock all of the futs that will be used */
		for (i1 = 0; i1 < evalControlP->nFuts; i1++) {
			fut = fut_lock_fut (evalControlP->evalList[i1]->data);
			if ((fut == NULL) || (fut_to_mft (fut) != 1)) {
				PTErr = KCP_PTERR_2;
				goto GetOut;
			}

			evalControlP->evalList[i1]->dataP = fut;
		}		

		fut = PTTableP->dataP;
		if ( ! IS_FUT(fut)) {
			PTErr = KCP_FAILURE;
			goto RetErr;
		}

		if (fut_is_separable (fut) == 1) {
			PTErr = KCP_SUCCESS;
			goto RetErr;
		}

		/* Find number of input variables */
		for (i2 = 0, numInputs = 0; i2 < FUT_NICHAN ; i2++) {
			if (fut->iomask.in & FUT_BIT(i2)) {
				iTblsP[numInputs] = fut->itbl[i2];	/* make a list of input tables */
				numInputs++;
			}
		}

		PTTableP->numInputs = numInputs;

		/* Find number of output channels */
		firstGTbl = NULL;
		for (i2 = 0, nOutputChans = 0; i2 < FUT_NOCHAN; i2++) {
			if (fut->iomask.out & FUT_BIT(i2)) {
				thisChan = fut->chan[i2];	/* this output channel is being evaluated */
				
				gTblsP[nOutputChans] = thisChan->gtbl->refTbl;	/* make a list of gtbls */
				oTblsP[nOutputChans] = thisChan->otbl;			/* and otbls */

				aGTbl = thisChan->gtbl;							/* shared input tables, so any gtbl will do */

				if (firstGTbl == NULL) {
					firstGTbl = aGTbl;							/* remember first gtbl */
				}

				/* all grids must have the same dimensionality */
				for (i1 = 0; i1 < FUT_NICHAN ; i1++) {
					if (aGTbl->size[i1] != firstGTbl->size[i1]) {
						PTErr = KCP_INVAL_EVAL;					/* sorry */
						goto RetErr;
					}
				}

				nOutputChans++;
			}
		}

		initGridInfo (numInputs, nOutputChans, PTTableP, firstGTbl);	/* set up grid table descriptors */

		#if defined KCP_SINGLE_EVAL_CACHE
		PTErr = getEvalStatePT (&evalStatePT);
		if (PTErr != KCP_SUCCESS) {
			goto RetErr;
		}

		if (evalStatePT != PTTableP->refNum) {
			freeEvalTables (evalStatePT);			/* retain a single set of optimized tables */
		}
		#endif
	}

	if (evalControlP->optimizedEval != 0) {	/* optimize tables */
		if (hasEtMem (etILutsP) == 1) {
			lockEtMem (etILutsP);			/* lock the required table */
		}
		else {
			totalInputLutBytes = inputTableEntries * evalControlP->iLutEntrySize * numInputs;

			allocEtMem (etILutsP, totalInputLutBytes);	/* allocate necessary memory for the input tables */
			if (hasEtMem (etILutsP) != 1) {
				goto ErrOut2;
			}

			PTTableP->optGridP = 0;			/* no grid table pointer the first time around */

			/* set up the input table for each variable
			/* this is done in 2 steps:
			/* expand (if necessary) the current input table to the size needed for the input data
			/* convert that table into a lut for evaluation
			 */

			iTblDat = allocBufferPtr (inputTableEntries * evalControlP->iLutEntrySize);

			theSizef = nOutputChans * evalControlP->gLutEntrySize;
#if defined KCP_PENTIUM
			if ((evalControlP->gLutFormat == ET_GTBL_FMT2) || (evalControlP->gLutFormat == ET_GTBL_FMT3)) {
				theSizef = 4 * evalControlP->gLutEntrySize;	/* dummy 4th channel used */
			}
#endif
			
			for (i2 = numInputs-1; i2 >= 0 ; i2--) {
				theITbl = iTblsP[i2];				/* input table to convert */

				convert1DTable (theITbl->refTbl, sizeof (mf2_tbldat_t), theITbl->refTblEntries, MF2_TBL_MAXVAL,
								iTblDat, sizeof (mf2_tbldat_t), inputTableEntries, MF2_TBL_MAXVAL,
								KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);

				iLuti = (evalILuti_p)((KpUInt8_p)etILutsP->P + (i2 * inputTableEntries * evalControlP->iLutEntrySize));	/* converted table */
				th1MFtbl2InLut (iTblDat, inputTableEntries, theITbl->size, theSizef, fracBits, evalControlP->iLutFormat, iLuti);	/* convert the input table to an eval lut */
				
				theSizef *= theITbl->size;
			}

			freeBufferPtr (iTblDat);
		}

		/* Grid Tables */
		/* are the grid tables already interleaved? */
		if (hasEtMem (etGLutsP) == 1) {
			lockEtMem (etGLutsP);	/* lock the required table */
		}
		else {
#if defined KCP_PENTIUM
			evalGLutSSE_p	gLutSSE;
			evalGLutSSE2_p	gLutSSE2;
#endif

			gridTblEntries = firstGTbl->tbl_size / sizeof (mf2_tbldat_t);
			totalGridTableBytes = gridTblEntries * nOutputChans * evalControlP->gLutEntrySize;	/* size needed for this fut's grid tables */
#if defined KCP_PENTIUM
			if ((evalControlP->gLutFormat == ET_GTBL_FMT2) || (evalControlP->gLutFormat == ET_GTBL_FMT3)) {
				totalGridTableBytes = gridTblEntries * 4 * evalControlP->gLutEntrySize;	/* size needed for this fut's grid tables */
			}
#endif

			allocEtMem (etGLutsP, totalGridTableBytes + SIZEOF_M128 -1);	/* allocate necessary memory for the grid tables */
			if (hasEtMem (etGLutsP) != 1) {						/* allow for 16-byte boundary */
				goto ErrOut2;
			}

			/* interleave the grid tables */
			interleavedGridP = (mf2_tbldat_p)etGLutsP->P;
#if defined KCP_PENTIUM
			gLutSSE = (evalGLutSSE_p)ALIGN_PTR(interleavedGridP, SIZEOF_M128);
			gLutSSE2 = (evalGLutSSE2_p)ALIGN_PTR(interleavedGridP, SIZEOF_M128);
#endif
			for (i2 = 0; i2 < gridTblEntries; i2++) {
				for (j = 0; j < nOutputChans; j++) {
					tmp = *gTblsP[j]++;
#if defined KCP_PENTIUM
					if (evalControlP->gLutFormat == ET_GTBL_FMT2) {
						*gLutSSE2++ = KCP_CONVERT_DOWN(tmp, MF2_TBL_BITS, endBits);
					}
					else if (evalControlP->gLutFormat == ET_GTBL_FMT3) {
						*gLutSSE++ = (float)tmp * gTableFactor;
					}
					else {
#endif
						tmp = KCP_CONVERT_DOWN(tmp, MF2_TBL_BITS, endBits);
						*interleavedGridP++ = tmp;
#if defined KCP_PENTIUM
					}
#endif
				}

#if defined KCP_PENTIUM
				if (evalControlP->gLutFormat == ET_GTBL_FMT2) {
					*gLutSSE2++ = 0;
				}
				else if (evalControlP->gLutFormat == ET_GTBL_FMT3) {	/* align to 16-byte boundary */
					*gLutSSE++ = (float)0;
				}
#endif
			}
		}

		/* Output Tables */
		if (hasEtMem (etOLutsP) == 1) {
			lockEtMem (etOLutsP);	/* lock the required table */
		}
		else {
			KpUInt8_p	oLuts;
		
			outputLutBytes = outputTableEntries * outputTableEntrySize;
			totalOutputLutBytes = outputLutBytes * nOutputChans;	/* size needed for this fut's output luts */

			allocEtMem (etOLutsP, totalOutputLutBytes);		/* allocate necessary memory for the output tables */
			if (hasEtMem (etOLutsP) != 1) {
				goto ErrOut2;
			}	

			/* set up the output table for each channel */
			oLuts = etOLutsP->P;

			for (i2 = 0; i2 < nOutputChans; i2++) {
				mf2_tbldat_p	srcTable;
				KpInt32_t		srcTableEntries;
				fut_otbl_p		otbl;

				otbl = oTblsP[i2];

				if ((IS_OTBL(otbl)) && (otbl->refTbl != NULL)) {
					srcTable = otbl->refTbl;
					srcTableEntries = otbl->refTblEntries;
				}
				else {
					srcTable = identityTable;
					srcTableEntries = 2;
				}
							
				convert1DTable (srcTable, sizeof (mf2_tbldat_t), srcTableEntries, MF2_TBL_MAXVAL,
								&oLuts [i2 * outputLutBytes], outputTableEntrySize, outputTableEntries, destTableMaxValue,
								KCP_MAP_END_POINTS, KCP_MAP_END_POINTS);
			}
		}

		/* add start of grid table into last input table */
		/* must be done after grid table is locked */
#if defined KCP_PENTIUM
		if ((evalControlP->gLutFormat == ET_GTBL_FMT2) || (evalControlP->gLutFormat == ET_GTBL_FMT3)) {
			KpInt32_t		delta, newOptGridP;
			evalILutSSE_p	iLutSSE;

			iLuti = (evalILuti_p)etILutsP->P;						/* start of table */
			iLutSSE = (evalILutSSE_p)etILutsP->P;

			/* remove old grid start and insert new */
			newOptGridP = (KpInt32_t)ALIGN_PTR(etGLutsP->P, SIZEOF_M128);
			delta = newOptGridP - PTTableP->optGridP;
			PTTableP->optGridP = newOptGridP;

			for (i2 = 0; i2 < inputTableEntries; i2++) {
				if (evalControlP->hasSSE == 1) {
					iLutSSE[i2].index += delta;
				}
				else if (evalControlP->hasSSE2 == 1) {
					iLuti[i2].index += delta;
				}
			}
		}
#endif

	}

	/* were tables built? */
GetOut:
	if ((PTErr == KCP_NO_MEMORY) || (PTMemTest () == 0)) {	/* switch to general eval function */
		evalControlP->optimizedEval = 0;	/* not optimized */
		evalControlP->evalFunc = evalTh1gen;
		PTErr = KCP_SUCCESS;
	}
	#if defined KCP_SINGLE_EVAL_CACHE
	else {									/* Update evaluation state info */
		if (PTErr == KCP_SUCCESS) {
			PTErr = putEvalStatePT (PTTableP->refNum);	/* remember which PT has the optimized tables */
		}
	}
	#endif

RetErr:
	KpLeaveCriticalSection (&PTCacheCritFlag);

	return PTErr;


ErrOut1:
	PTErr = KCP_INVAL_EVAL;
	goto ErrOut;
	
ErrOut2:	
	PTErr = KCP_NO_MEMORY;

ErrOut:	
	freeEvalTables (PTTableP->refNum);	/* free whatever may have been allocated */
	goto GetOut;
}



/******************************************************************************/
/* Setup up grid tables offsets if necessary  */
static void
	initGridInfo (	KpInt32_t		numInputs,
					KpInt32_t		nOutputChans,
					PTTable_p		PTTableP,
					fut_gtbl_p		aGTbl)
{
KpInt32_t	theSizef, i2, dimx, dimy, dimz;

	/* set up offsets in grid */
	switch (numInputs) {
	case 1:
		dimx = 0;	/* not used */
		dimy = 0;
		dimz = 0;
		break;

	case 2:
		dimx = 0;	/* not used */
		dimy = 0;
		dimz = aGTbl->size[1];
		break;

	case 3:
		dimx = 0;	/* not used */
		dimy = aGTbl->size[1];
		dimz = aGTbl->size[2];
		break;

	case 4:
		dimx = aGTbl->size[1];
		dimy = aGTbl->size[2];
		dimz = aGTbl->size[3];
		break;

	default:
		return;
	}		
		
	PTTableP->etGOffsets[0] = 0;													/* offset to 0000 */
	PTTableP->etGOffsets[1] = 1;													/* offset to 0001 */
	PTTableP->etGOffsets[2] = dimz;													/* offset to 0010 */
	PTTableP->etGOffsets[3] = PTTableP->etGOffsets[2] + 1;							/* offset to 0011 */
	PTTableP->etGOffsets[4] = dimy * PTTableP->etGOffsets[2];						/* offset to 0100 */
	PTTableP->etGOffsets[5] = PTTableP->etGOffsets[4] + 1;							/* offset to 0101 */
	PTTableP->etGOffsets[6] = PTTableP->etGOffsets[4] + PTTableP->etGOffsets[2];	/* offset to 0110 */
	PTTableP->etGOffsets[7] = PTTableP->etGOffsets[6] + 1;							/* offset to 0111 */
	PTTableP->etGOffsets[8] = dimx * PTTableP->etGOffsets[4];						/* offset to 1000 */
	PTTableP->etGOffsets[9] = PTTableP->etGOffsets[8] + 1;							/* offset to 1001 */
	PTTableP->etGOffsets[10] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[2];	/* offset to 1010 */
	PTTableP->etGOffsets[11] = PTTableP->etGOffsets[10] + 1;						/* offset to 1011 */
	PTTableP->etGOffsets[12] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[4];	/* offset to 1100 */
	PTTableP->etGOffsets[13] = PTTableP->etGOffsets[12] + 1;						/* offset to 1101 */
	PTTableP->etGOffsets[14] = PTTableP->etGOffsets[8] + PTTableP->etGOffsets[4] + PTTableP->etGOffsets[2];	/* offset to 1110 */
	PTTableP->etGOffsets[15] = PTTableP->etGOffsets[14] + 1;						/* offset to 1111 */

	theSizef = nOutputChans * sizeof (mf2_tbldat_t);
	for (i2 = 0; i2 < TH1_NUM_OFFSETS; i2++) {
		PTTableP->etGOffsets[i2] *= theSizef;	/* adjust for # and size of channels in interleaved grid tables */
	}
	
	return;
}


/******************************************************************************/
/* Convert fut itbl to input table in special format */
static void
	th1MFtbl2InLut (	mf2_tbldat_p	futLut,
						KpInt32_t		nEntries,
						KpInt32_t		gridDim,
						KpInt32_t		sizef,
						KpInt32_t		fracBits,
						KpUInt32_t		lutType,
						KpGenericPtr_t	evalLut)
{
KpInt32_t	i2, index, v, maxDim, fracMask;
KpFloat32_t	factor;
KpUInt32_t	frac;
evalILuti_p		evalLuti = evalLut;
#if defined KCP_PENTIUM
evalILutSSE_p	evalLutSSE = evalLut;
evalILutSSE2_p	evalLutSSE2 = evalLut;
#endif

	fracMask = (1 << fracBits) -1;	/* mask of fractional bits */

	/* Convert the itbl value to values convenient for grid table interpolation. */
	maxDim = gridDim -1;

	factor = ((KpFloat32_t) ((gridDim -1) * (1 << fracBits))) / MF2_TBL_MAXVAL;
	
	for (i2 = 0; i2 < nEntries; i2++) {
		v = (KpInt32_t) ((futLut[i2] * factor) + 0.5);	/* get fut lut value */
		
		index = v >> fracBits;		/* get fut integer index value */
		frac = v & fracMask;	 	/* get fut fractional value */

/* George added this in when doing the change to add SSE/SSE2 processing */
/* Removed this for now since it breaks when the input table contains */
/* data that is decreasing */
/*		if (i2 == 0) {				/* ensure 0 index for first entry */
/*			index = 0;				/* required to be able to put grid table start into Z index */
/*		} */

		if (index == gridDim -1) {	/* prevent addressing past end of table */
			frac = fracMask;
			index--;
		}

		switch (lutType) {
#if defined KCP_PENTIUM
		case ET_ITBL_FMT3:
			evalLutSSE[i2].index = index * sizef;	/* position index for offset into fut grid */
			evalLutSSE[i2].frac = ((float)frac) / (1 << fracBits);
			break;
#endif
		default:
			evalLuti[i2].index = index * sizef;
			evalLuti[i2].frac = frac;
		}
	}
}


/******************************************************************************/
/* Deallocate all allocated memory and null out handles. */
/* Note: will deallocate properly if only some of the memory got allocated. */
void
	freeEvalTables (	PTRefNum_t	PTRefNum)
{
PTTable_p	PTTableP;
KpInt32_t	i1;

#if defined KCP_SINGLE_EVAL_CACHE
	putEvalStatePT (NULL);
#endif

	PTTableP = lockPTTable (PTRefNum);

	if (PTTableP == NULL) {
		return;
	}

	for (i1 = 0; i1 < ET_NLUT; i1++) {
		freeEtMem (&PTTableP->etLuts[i1]);
	}

	nullEvalTables (PTTableP);			/* set evaluation state to null */

	unlockPTTable (PTRefNum);
}


/******************************************************************************/
/* Unlock the evaluation state */

void
	unlockEvalTables (	PTTable_p	PTTableP)
{
KpInt32_t	i1;

	if (PTTableP != NULL) {

		for (i1 = 0; i1 < ET_NLUT; i1++) {
			unlockEtMem (&PTTableP->etLuts[i1]);
		}
	}
}


/******************************************************************************/
/* Set the evaluation state to unused */
void
	nullEvalTables (	PTTable_p	PTTableP)
{
KpInt32_t	i1;

	if (PTTableP == NULL) {
		return;
	}

	for (i1 = 0; i1 < ET_NLUT; i1++) {
		nullEtMem (&PTTableP->etLuts[i1]);
	}

	return;
}


/******************************************************************************/
/* allocate evaluation state memory */
static void
	allocEtMem (etMem_p		theEtMemP,
				KpInt32_t	bytesNeeded)
{
	/* is memory already allocated and the correct size? */
	if (theEtMemP->bytes != bytesNeeded) {
		freeEtMem (theEtMemP);							/* free current allocation */
		
		theEtMemP->P = allocBufferPtr (bytesNeeded);	/* get new memory */
		if (theEtMemP->P != NULL) {
			theEtMemP->bytes = bytesNeeded;
			theEtMemP->lockCount = 1;
		}
	}
}


/******************************************************************************/
/* free evaluation state memory */
static void
	freeEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP->P != NULL) {
		freeBufferPtr (theEtMemP->P);
	} else if (theEtMemP->H != NULL) {
		freeBuffer (theEtMemP->H);
 	}
	nullEtMem (theEtMemP);
}


/******************************************************************************/
/* check for evaluation state memory */
static KpInt32_t
	hasEtMem (etMem_p	theEtMemP)
{
	if ((theEtMemP->P != NULL) | (theEtMemP->H != NULL)) {
		return 1;
	}
	else {
		return 0;
	}
}


/******************************************************************************/
/*  set evaluation state memory to null */
static void
	nullEtMem (etMem_p	theEtMemP)
{
	theEtMemP->H = NULL;
	theEtMemP->P = NULL;
	theEtMemP->bytes = 0;
	theEtMemP->lockCount = 0;
}


/******************************************************************************/
/* lock evaluation state memory */
static void
	lockEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP != NULL) {
		if (theEtMemP->H != NULL) {
			theEtMemP->P = (etMem_p) lockBuffer (theEtMemP->H);
			theEtMemP->H = NULL;	/* indicate locked */
		}
		theEtMemP->lockCount++;
	}
}


/******************************************************************************/
/* unlock evaluation state memory */
static void
	unlockEtMem (etMem_p	theEtMemP)
{
	if (theEtMemP != NULL) {
		if (theEtMemP->P != NULL) {
			theEtMemP->lockCount--;
			if (0 == theEtMemP->lockCount) {
				theEtMemP->H = getHandleFromPtr(theEtMemP->P);	/* get handle */
				(void) unlockBuffer (theEtMemP->H);
				theEtMemP->P = NULL;							/* forget the pointer */
			}
		}
	}
}


#if defined KCP_SINGLE_EVAL_CACHE

/* get the current chaining state from thread globals */
static PTErr_t
	getEvalStatePT (	PTRefNum_p	evalStatePT)
{
processGlobals_p	pGP;

	pGP = loadProcessGlobals();
	if (pGP == NULL) {
		return	KCP_NO_PROCESS_GLOBAL_MEM;
	}

	*evalStatePT = pGP->evalStatePT;

	unloadProcessGlobals ();

	return KCP_SUCCESS;
}


/* put the current chaining state in the thread globals */
static PTErr_t
	putEvalStatePT (	PTRefNum_t	evalStatePT)
{
processGlobals_p	pGP;

	pGP = loadProcessGlobals();
	if (pGP == NULL) {
		return	KCP_NO_PROCESS_GLOBAL_MEM;
	}

	pGP->evalStatePT = evalStatePT;

	unloadProcessGlobals ();

	return KCP_SUCCESS;
}

#endif
