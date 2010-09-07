/*
 * @(#)evalth1g.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	evalth1g.c

	Contains:	general tetrahedral evaluation function

	Author:		George Pawle

	COPYRIGHT (c) 1997-2000 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
 */

#include "kcpcache.h"
#include "fut_util.h"

#define EVAL_HIGH_MASK (0xfffff800)
#define EVAL_EXTENDED_BITS (8)
#define EVAL_REMAINDER_BITS (15)
#define ROUND_VALUE(bits) ((1 << (bits -1)) -1)

typedef struct division_s {
	KpUInt32_t	quotient;
	KpUInt32_t	remainder;
} division_t, FAR* division_p;

/* Bose sort pairs */
static KpInt32_t BoseSort1[] = {0};
static KpInt32_t BoseSort2[] = {1, 0, 1};
static KpInt32_t BoseSort3[] = {3, 0, 1, 0, 2, 1, 2};
static KpInt32_t BoseSort4[] = {5, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2};
static KpInt32_t BoseSort5[] = {9, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 0, 4, 1, 4, 2, 4, 3, 4};
static KpInt32_t BoseSort6[] = {12, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 4, 5, 0, 4, 1, 5, 1, 4, 2, 4, 3, 5, 3, 4};
static KpInt32_t BoseSort7[] = {16, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 4, 5, 4, 6, 5, 6, 0, 4, 1, 5, 1, 4, 2, 6, 3, 6, 2, 4, 3, 5, 3, 4};
static KpInt32_t BoseSort8[] = {19, 0, 1, 2, 3, 0, 2, 1, 3, 1, 2, 4, 5, 6, 7, 4, 6, 5, 7, 5, 6, 0, 4, 1, 5, 1, 4, 2, 6, 3, 7, 3, 6, 2, 4, 3, 5, 3, 4};

/******************************************************************************/

#define doDivide(dividend, divisor, result) \
{ \
KpUInt32_t	dividendShifted; \
 \
	dividendShifted = (dividend) << EVAL_D16_FRACBITS;				/* get extra precision */ \
	result.quotient = dividendShifted / (divisor); \
	result.remainder = (((dividendShifted - (result.quotient * (divisor))) << EVAL_REMAINDER_BITS) + ((divisor) > 1) -1) / (divisor); \
}


#define doMultiply(multiplier, multiplicand, result) \
{ \
KpUInt32_t	correction; \
 \
	result = (multiplier) * multiplicand.quotient; \
	correction = (((multiplier) * multiplicand.remainder) + ROUND_VALUE(EVAL_REMAINDER_BITS)) >> EVAL_REMAINDER_BITS; \
	result += correction; \
}


#define interpolateDelta(tableData, nextTableData, interpolant, result) \
{ \
KpInt32_t	delta, highBits, deltaHigh, resultH; \
 \
	delta = nextTableData - tableData; \
 \
	highBits = delta & EVAL_HIGH_MASK; \
 \
	if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
	 	KCP_SHIFT_RIGHT(delta, deltaHigh, EVAL_EXTENDED_BITS) \
		resultH = interpolant * deltaHigh; \
	 	delta &= ((1 << EVAL_EXTENDED_BITS) -1); \
		result = interpolant * delta; \
	 	result += ROUND_VALUE(EVAL_EXTENDED_BITS); \
 		KCP_SHIFT_RIGHT(result, result, EVAL_EXTENDED_BITS)	/* discard unneeded low order bits */ \
	 	result += resultH;			/* add in high order bits */ \
	} \
	else { \
		result = interpolant * delta; \
	 	result += ROUND_VALUE(EVAL_EXTENDED_BITS); \
		KCP_SHIFT_RIGHT(result, result, EVAL_EXTENDED_BITS); \
	} \
}



static KpInt32_t
	interp1DTable (	mf2_tbldat_p	the1DTable,
					KpInt32_t		tableEntries,
					KpInt32_t		srcData,
					division_t		dataFactor)
{
KpUInt32_t	sPosition, index;
KpInt32_t	interpolant, tableData, nextTableData;

	doMultiply (srcData, dataFactor, sPosition);	/* calculate the input table position */
	index = sPosition >> EVAL_D16_FRACBITS;
	interpolant = sPosition & ((1 << EVAL_D16_FRACBITS) -1);

	tableData = (KpInt32_t) the1DTable[index];			/* pass input data through input tables */

	if (index < tableEntries -1) {
		nextTableData = (KpInt32_t) the1DTable[index +1];	/* next table data */

		interpolateDelta (tableData, nextTableData, interpolant, nextTableData)
		nextTableData += ROUND_VALUE(EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS);
		KCP_SHIFT_RIGHT(nextTableData, nextTableData, EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS);

		tableData += nextTableData;
	}
	
	return tableData;
}


/******************************************************************************/

void
	evalTh1gen (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
imagePtr_t		inData[FUT_NICHAN], outData[FUT_NOCHAN];
KpInt32_t		inStrideL[FUT_NICHAN], outStrideL[FUT_NOCHAN];
KpInt32_t		i1, separableFut, numInputs, numOutputs, gDimSize[FUT_NOCHAN], oTblEntries[FUT_NOCHAN];
division_t		iIndexFactor[FUT_NICHAN], gIndexFactor[FUT_NICHAN], oIndexFactor[FUT_NOCHAN];
KpInt32_t		oDataShift, oDataRound, oDataFactor, dataMax, oDataBits;
fut_p			fut;
fut_itbl_p		iTbl[FUT_NICHAN], theITbl;
fut_chan_p		chan[FUT_NOCHAN], theChan;
mf2_tbldat_p	gTbl[FUT_NOCHAN], oTbl[FUT_NOCHAN];
KpInt32_p		BoseSort[FUT_NICHAN] = {BoseSort1, BoseSort2, BoseSort3, BoseSort4, BoseSort5, BoseSort6, BoseSort7, BoseSort8};
mf2_tbldat_t	identityTable[2] = {0, MF2_TBL_MAXVAL};

	fut = PTTableP->dataP;

	separableFut = fut_is_separable (fut);		/* check for separable (linearization) fut */

	/* set up input table stuff */
	switch (dataTypeI) {
	case KCM_UBYTE:
		dataMax = (1 << 8) -1;
		break;

	case KCM_USHORT_12:
		dataMax = (1 << 12) -1;
		break;

	case KCM_USHORT:
		dataMax = (1 << 16) -1;
		break;
		
	default:
		dataMax = 1;
	}

	for (i1 = 0, numInputs = 0; i1 < FUT_NICHAN; i1++) {
		if (inp[i1].p8 != NULL) {
			inData[numInputs].p8 = inp[i1].p8;	/* copy addresses - do not change supplied lists! */
			inStrideL[numInputs] = inStride[i1];

			theITbl = fut->itbl[i1];
			if ( ! IS_ITBL(theITbl)) {
				return;
			}

			iTbl[numInputs] = theITbl;					/* collect the input tables */

			/* set up interpolation into input table */
			doDivide (theITbl->refTblEntries -1, dataMax, iIndexFactor[numInputs]);	/* set up interpolation into input table */

			/* set up interpolation into grid table */
			gDimSize[i1] = theITbl->size;	/* save in case of separable fut */
			doDivide (gDimSize[i1] -1, MF2_TBL_MAXVAL, gIndexFactor[numInputs]);	/* set up interpolation into input table */
			
			numInputs++;
		}
	}

	/* set up grid and output table stuff */
	for (i1 = 0, numOutputs = 0; i1 < FUT_NOCHAN; i1++) {
		if (outp[i1].p8 != NULL) {
			fut_otbl_p	theOTbl;

			outData[numOutputs].p8 = outp[i1].p8;	/* copy addresses - do not update supplied lists! */
			outStrideL[numOutputs] = outStride[i1];

			theChan = fut->chan[i1];
			if ( ! IS_CHAN(theChan)) {
				return;
			}

			chan[numOutputs] = theChan;

			gTbl[numOutputs] = theChan->gtbl->refTbl;	/* get the grid */

			theOTbl = theChan->otbl;		/* set up interpolation into output table */
			if ( ! IS_OTBL(theOTbl) || ((oTbl[numOutputs] = theOTbl->refTbl) == NULL)) {
				oTbl[numOutputs] = identityTable;
				oTblEntries[numOutputs] = 2;
			}
			else {
				oTblEntries[numOutputs] = theOTbl->refTblEntries;
			}

			doDivide (oTblEntries[numOutputs] -1, MF2_TBL_MAXVAL, oIndexFactor[numOutputs]);	/* set up interpolation into input table */

			numOutputs++;
		}
	}
	
	/* set up output data scaling */
	switch (dataTypeO) {
	case KCM_UBYTE:
		oDataBits = 8;
		break;

	case KCM_USHORT_12:
		oDataBits = 12;
		break;

	case KCM_USHORT:
		oDataBits = 16;
		break;
		
	default:
		dataMax = 1;
	}

	dataMax = (1 << oDataBits) -1;
	oDataShift = 32 -1 - oDataBits;
	oDataFactor = (dataMax << oDataShift) / MF2_TBL_MAXVAL;
	oDataRound = (1 << (oDataShift -1)) -1;


	/* all set up; evaluate each pixel */
	for (i1 = 0; i1 < n; i1++) {
		KpInt32_t	cell, i2, dimSize, numCompares, hVert[FUT_NICHAN];
		KpInt32_t	iTableData[FUT_NICHAN], hFrac[FUT_NICHAN];
		KpUInt32_t	sPosition, index;
		KpInt32_p	BoseSortP;

		for (i2 = 0, cell = 0; i2 < numInputs; i2++) {
			KpInt32_t	srcData, interpData;

			if (dataTypeI == KCM_UBYTE) {
				srcData = (KpInt32_t) (*inData[i2].p8); 	/* get 8 bit input data */
			}
			else {
				srcData = (KpInt32_t) (*inData[i2].p16); 	/* get 12/16 bit input data */
			}
						
			inData[i2].p8 += inStrideL[i2];

			/* pass source image data through the input table */
			theITbl = iTbl[i2];
			interpData = interp1DTable (theITbl->refTbl, theITbl->refTblEntries, srcData, iIndexFactor[i2]);
			iTableData[i2] = interpData;	/* save in case of separable fut */

			doMultiply (interpData, gIndexFactor[i2], sPosition);	/* calculate the grid table position */
			index = sPosition >> EVAL_D16_FRACBITS;

			dimSize = theITbl->size;					/* size of this dimension */

			if (index < dimSize -1) {
				hFrac[i2] = sPosition & ((1 << EVAL_D16_FRACBITS) -1);	/* get grid interpolant */
			}
			else {
				hFrac[i2] = (1 << EVAL_D16_FRACBITS) -1;
				index--;
			}

			hVert[i2] = dimSize;		/* save for offset calcs */
			cell *= dimSize;			/* build cell index */
			cell += index;				/* add in this index */
		}

		/* build offsets for each dimension */
		index = 2;
		for (i2 = numInputs-1; i2 >= 0; i2--) {
			dimSize = hVert[i2];
			hVert[i2] = index;
			index *= dimSize;
		}

		/* find the hyperhedron in which the interpolation point is located */
		BoseSortP = BoseSort[numInputs -1];
		numCompares = *BoseSortP++;		/* first element is # of compares */
		
		for (i2 = 0; i2 < numCompares; i2++) {
			KpInt32_t	tmpI, index1, index2;

			index1 = *BoseSortP++;
			index2 = *BoseSortP++;
			
			/* sort into largest to smallest based upon interpolants */
			tmpI = hFrac[index1];
			if (tmpI < hFrac[index2]) {
				hFrac[index1] = hFrac[index2];	/* swap interpolants */
				hFrac[index2] = tmpI;

				tmpI = hVert[index1];			/* swap vertices */
				hVert[index1] = hVert[index2];
				hVert[index2] = tmpI;
			}
		}

		/* evaluate each output channel */
		for (i2 = 0; i2 < numOutputs; i2++) {
			KpInt32_t	i3, tResult, oTableData, previousVertex, thisVertex;
			KpUInt8_p	vertexP;

			if (separableFut == 1) {
				tResult = interp1DTable (gTbl[i2], gDimSize[i2], iTableData[i2], gIndexFactor[i2]);
			}
			else {		/* hyperhedral interpolation */		
				vertexP = (KpUInt8_p)(gTbl[i2] + cell);

				previousVertex = (KpInt32_t) *(mf2_tbldat_p)(vertexP);
				tResult = previousVertex << (EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS);

				for (i3 = 0; i3 < numInputs; i3++) {
					vertexP += hVert[i3];
					thisVertex = (KpInt32_t) *(mf2_tbldat_p)(vertexP);

					interpolateDelta (previousVertex, thisVertex, hFrac[i3], previousVertex)
					tResult += previousVertex;

					previousVertex = thisVertex;
				}

				tResult += ROUND_VALUE(EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS);
				KCP_SHIFT_RIGHT(tResult, tResult, EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS);
			}

			/* output table lookup */
			oTableData = interp1DTable (oTbl[i2], oTblEntries[i2], tResult, oIndexFactor[i2]);
			oTableData *= oDataFactor;		/* convert to dest size */
			oTableData += oDataRound;		/* round */
			oTableData >>= oDataShift;		/* remove fractional bits */

			if (dataTypeO == KCM_UBYTE) {
				*outData[i2].p8 = (KpUInt8_t) oTableData;
			}
			else {
				*outData[i2].p16 = (KpUInt16_t) oTableData;
			}
			
			outData[i2].p8 += outStrideL[i2];	/* next output data location */
		}
	}
}
