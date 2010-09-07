/*
 * @(#)evalth14.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  evalth14.c 
*
*	4 input evaluation functions using tetrahedral interpolation
*
*	Author:			George Pawle
*
*	Creation Date:	12/22/96
*
*    COPYRIGHT (c) 1996-2003 Eastman Kodak Company
*    As an unpublished work pursuant to Title 17 of the United
*    States Code.  All rights reserved.
*/

#include "kcpcache.h"

#define EVAL_HIGH_MASK (0xfffffe00)
#define EVAL_EXTENDED_BITS (8)

#define TH1_4D_OCHAN_VARS(bits, chan) \
KpUInt8_p			gridBase##chan; \
KpInt32_t			outStride##chan; \
KpUInt##bits##_p	outLut##chan, outp##chan;

#define TH1_4D_INIT_VARS \
KpUInt8_p	gridBaseP, thisOutLut; \
evalILuti_p	inLut0; \
KpUInt32_t	srcData0, srcData1, srcData2, srcData3, a[TH1_NUM_OFFSETS]; \
KpInt32_t	i1, oChan, tResult, tvert1, tvert2, tvert3, tvert4, mullacc, temp, tvert1Data, tvert2Data, tvert3Data, tvert4Data; \
KpInt32_t	inStride0 = inStride[0], inStride1 = inStride[1], inStride2 = inStride[2], inStride3 = inStride[3]; \
KpInt32_t	baseOffset, Xf, Yf, Zf, Tf, tfrac1, tfrac2, tfrac3, tfrac4;

#define TH1_4D_INIT_VARS_D8 \
KpInt32_t	outLutSize = EVAL_OLUT_ENTRIESD8; \
KpUInt8_p	inp0 = inp[0].p8, inp1 = inp[1].p8, inp2 = inp[2].p8, inp3 = inp[3].p8;

#define TH1_4D_INIT_VARS_D16 \
KpInt32_t	outLutSize = EVAL_OLUT_ENTRIESD16 * sizeof (KpUInt16_t); \
KpUInt16_p	inp0 = inp[0].p16, inp1 = inp[1].p16, inp2 = inp[2].p16, inp3 = inp[3].p16; \
evalILuti_p	inLut1, inLut2, inLut3; \
KpUInt32_t	dataMask; \
KpInt32_t	iLutEntries, delta3, delta2, delta1, delta0, deltaHigh, mullaccH, highBits;

#define TH1_4D_INIT_TABLES_D8 \
	if (dataTypeI) {}  \
	if (dataTypeO) {}  \
	inLut0 = PTTableP->etLuts[ET_I8].P; \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G12F1].P; \
	thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O8].P;

#define TH1_4D_INIT_TABLES_D16 \
	if (dataTypeI == KCM_USHORT_12) { \
		iLutEntries = 1 << 12; \
		inLut0 = PTTableP->etLuts[ET_I12F1].P; \
		gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	} \
	else { \
		iLutEntries = 1 << 16; \
		inLut0 = PTTableP->etLuts[ET_I16].P; \
		gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	} \
 \
	inLut1 = inLut0 + iLutEntries; \
	inLut2 = inLut1 + iLutEntries; \
	inLut3 = inLut2 + iLutEntries; \
	dataMask = iLutEntries -1;	/* set up data mask to prevent input table memory access violations */ \
 \
	if (dataTypeO == KCM_USHORT_12) { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O12].P; \
	} \
	else { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O16].P; \
	}

#define TH1_4D_INIT_DATA \
	a[1] = PTTableP->etGOffsets[1];		/* copy grid offsets into locals */ \
	a[2] = PTTableP->etGOffsets[2]; \
	a[3] = PTTableP->etGOffsets[3]; \
	a[4] = PTTableP->etGOffsets[4]; \
	a[5] = PTTableP->etGOffsets[5]; \
	a[6] = PTTableP->etGOffsets[6]; \
	a[7] = PTTableP->etGOffsets[7]; \
	a[8] = PTTableP->etGOffsets[8]; \
	a[9] = PTTableP->etGOffsets[9]; \
	a[10] = PTTableP->etGOffsets[10]; \
	a[11] = PTTableP->etGOffsets[11]; \
	a[12] = PTTableP->etGOffsets[12]; \
	a[13] = PTTableP->etGOffsets[13]; \
	a[14] = PTTableP->etGOffsets[14]; \
	tvert4 = PTTableP->etGOffsets[15]; \
 \
	oChan = -1; \
	gridBaseP -= sizeof (mf2_tbldat_t); \
	thisOutLut -= outLutSize;

#define TH1_4D_OCHAN_TABLES(obits, chan) \
	do {	/* set up output tables */ \
		oChan++; \
		gridBaseP += sizeof (mf2_tbldat_t); \
		thisOutLut += outLutSize; \
		if (outp[oChan].p##obits != NULL) {	/* this output channel is being evaluated */ \
			break; \
		} \
	} while (1); \
	outp##chan = outp[oChan].p##obits; \
	outStride##chan = outStride[oChan]; \
	gridBase##chan = gridBaseP; \
	outLut##chan = (KpUInt##obits##_p)thisOutLut;

#define TH1_4D_INIT(bits) \
	TH1_4D_INIT_VARS \
	TH1_4D_INIT_VARS_D##bits \
	TH1_4D_INIT_TABLES_D##bits \
	TH1_4D_INIT_DATA

#define TH1_4D_GET_TETRA_D16 \
	srcData0 = *inp0; 						/* get channel 0 input data */ \
	srcData0 &= dataMask; \
	inp0 = (KpUInt16_p)((KpUInt8_p)inp0 + inStride0); \
	baseOffset = inLut0[srcData0].index;	/* pass input data through input tables */ \
	Xf = inLut0[srcData0].frac; \
	srcData1 = *inp1; 						/* channel 1 */ \
	srcData1 &= dataMask; \
	inp1 = (KpUInt16_p)((KpUInt8_p)inp1 + inStride1); \
	baseOffset += inLut1[srcData1].index; \
	Yf = inLut1[srcData1].frac; \
	srcData2 = *inp2; 						/* channel 2 */ \
	srcData2 &= dataMask; \
	inp2 = (KpUInt16_p)((KpUInt8_p)inp2 + inStride2); \
	baseOffset += inLut2[srcData2].index; \
	Zf = inLut2[srcData2].frac; \
	srcData3 = *inp3; 						/* channel 3 */ \
	srcData3 &= dataMask; \
	inp3 = (KpUInt16_p)((KpUInt8_p)inp3 + inStride3); \
	baseOffset += inLut3[srcData3].index; \
	Tf = inLut3[srcData3].frac; \
\
	TH1_4D_CALC_TETRA


#define TH1_4D_GET_TETRA_D8 \
	srcData0 = *inp0; 					/* get channel 0 input data */ \
	inp0 += inStride0;					/* next data address */ \
	baseOffset = inLut0[(0*FUT_INPTBL_ENT) + srcData0].index; 	/* pass input data through input tables */ \
	Xf = inLut0[(0*FUT_INPTBL_ENT) + srcData0].frac; \
	srcData1 = *inp1; 					/* channel 1 */ \
	inp1 += inStride1; \
	baseOffset += inLut0[(1*FUT_INPTBL_ENT) + srcData1].index; \
	Yf = inLut0[(1*FUT_INPTBL_ENT) + srcData1].frac; \
	srcData2 = *inp2; 					/* channel 2 */ \
	inp2 += inStride2; \
	baseOffset += inLut0[(2*FUT_INPTBL_ENT) + srcData2].index; \
	Zf = inLut0[(2*FUT_INPTBL_ENT) + srcData2].frac; \
	srcData3 = *inp3; 					/* channel 3 */ \
	inp3 += inStride3; \
	baseOffset += inLut0[(3*FUT_INPTBL_ENT) + srcData3].index; \
	Tf = inLut0[(3*FUT_INPTBL_ENT) + srcData3].frac; \
\
	TH1_4D_CALC_TETRA


#define TH1_4D_CALC_TETRA_BOSE \
KpInt32_t	index, Xi, Yi, Zi, Ti, tmp; \
	/* find the pentahedron in which the point is located */ \
	/* this orders the interpolants and increments using a Bose Sort */ \
	Xi = 8; \
	Yi = 4; \
	Zi = 2; \
	Ti = 1; \
	if (Xf < Yf) { \
		tmp = Xf; \
		Xf = Yf; \
		Yf = tmp; \
		tmp = Xi; \
		Xi = Yi; \
		Yi = tmp; \
	} \
	if (Zf < Tf) { \
		tmp = Zf; \
		Zf = Tf; \
		Tf = tmp; \
		tmp = Zi; \
		Zi = Ti; \
		Ti = tmp; \
	} \
	if (Xf < Zf) { \
		tmp = Xf; \
		Xf = Zf; \
		Zf = tmp; \
		tmp = Xi; \
		Xi = Zi; \
		Zi = tmp; \
	} \
	if (Yf < Tf) { \
		tmp = Yf; \
		Yf = Tf; \
		Tf = tmp; \
		tmp = Yi; \
		Yi = Ti; \
		Ti = tmp; \
	} \
	if (Yf < Zf) { \
		tmp = Yf; \
		Yf = Zf; \
		Zf = tmp; \
		tmp = Yi; \
		Yi = Zi; \
		Zi = tmp; \
	} \
	tfrac1 = Xf; \
	tfrac2 = Yf; \
	tfrac3 = Zf; \
	tfrac4 = Tf; \
	index = Xi; \
	tvert1 = a[index]; \
	index += Yi; \
	tvert2 = a[index]; \
	index += Zi; \
	tvert3 = a[index];

#define XI (1 << 3)
#define YI (1 << 2)
#define ZI (1 << 1)
#define TI (1 << 0)

#define TH1_VERTEX(vert, dim, inc) \
	tfrac##vert = dim##f; \
	tvert##vert = a[inc + dim##I];

#define TH1_4D_CALC_TETRA_XYZT \
	TH1_VERTEX(1, X) \
	TH1_VERTEX(2, Y) \
	TH1_VERTEX(3, Z) \
	tfrac4 = Tf;

/* find the pentahedron in which the point is located */
/* uses the Bose sort to determine the ordering, then sets fractions and increments directly */
/* talk about throwing code at the problem! */
#define TH1_4D_CALC_TETRA \
	if (Xf > Yf) { \
		if (Zf > Tf) { \
			if (Xf > Zf) { \
				TH1_VERTEX(1, X, 0) \
				if (Yf > Tf) { \
					if (Yf > Zf) {		/* xyzt */ \
						TH1_VERTEX(2, Y, XI) \
						TH1_VERTEX(3, Z, XI+YI) \
					} \
					else {				/* xzyt */ \
						TH1_VERTEX(2, Z, XI) \
						TH1_VERTEX(3, Y, XI+ZI) \
					} \
					tfrac4 = Tf; \
				} \
				else {					/* xzty */ \
					TH1_VERTEX(2, Z, XI) \
					TH1_VERTEX(3, T, XI+ZI) \
					tfrac4 = Yf; \
				} \
			} \
			else { \
				TH1_VERTEX(1, Z, 0) \
				if (Yf > Tf) {			/* zxyt */ \
					TH1_VERTEX(2, X, ZI) \
					TH1_VERTEX(3, Y, ZI+XI) \
					tfrac4 = Tf; \
				}  \
				else { \
					if (Tf > Xf) {		/* ztxy */ \
						TH1_VERTEX(2, T, ZI) \
						TH1_VERTEX(3, X, ZI+TI) \
					} \
					else {				/* zxty */ \
						TH1_VERTEX(2, X, ZI) \
						TH1_VERTEX(3, T, ZI+XI) \
					} \
					tfrac4 = Yf; \
				} \
			} \
		} \
		else { \
			if (Xf > Tf) { \
				TH1_VERTEX(1, X, 0) \
				if (Yf > Zf) { \
					if (Yf > Tf) {		/* xytz */ \
						TH1_VERTEX(2, Y, XI) \
						TH1_VERTEX(3, T, XI+YI) \
					} \
					else {				/* xtyz */ \
						TH1_VERTEX(2, T, XI) \
						TH1_VERTEX(3, Y, XI+TI) \
					} \
					tfrac4 = Zf; \
				} \
				else {					/* xtzy */ \
					TH1_VERTEX(2, T, XI) \
					TH1_VERTEX(3, Z, XI+TI) \
					tfrac4 = Yf; \
				} \
			} \
			else { \
				TH1_VERTEX(1, T, 0) \
				if (Yf > Zf) {			/* txyz */ \
					TH1_VERTEX(2, X, TI) \
					TH1_VERTEX(3, Y, TI+XI) \
					tfrac4 = Zf; \
				} \
				else { \
					if (Zf > Xf) {		/* tzxy */ \
						TH1_VERTEX(2, Z, TI) \
						TH1_VERTEX(3, X, TI+ZI) \
					} \
					else {				/* txzy */ \
						TH1_VERTEX(2, X, TI) \
						TH1_VERTEX(3, Z, TI+XI) \
					} \
					tfrac4 = Yf; \
				} \
			} \
		} \
	} \
	else { \
		if (Zf > Tf) { \
			if (Yf > Zf) { \
				TH1_VERTEX(1, Y, 0) \
				if (Xf > Tf) { \
					if (Xf > Zf) {		/* yxzt */ \
						TH1_VERTEX(2, X, YI) \
						TH1_VERTEX(3, Z, YI+XI) \
					} \
					else {				/* yzxt */ \
						TH1_VERTEX(2, Z, YI) \
						TH1_VERTEX(3, X, YI+ZI) \
					} \
					tfrac4 = Tf; \
				} \
				else { 					/* yztx */ \
					TH1_VERTEX(2, Z, YI) \
					TH1_VERTEX(3, T, YI+ZI) \
					tfrac4 = Xf; \
				} \
			} \
			else { \
				TH1_VERTEX(1, Z, 0) \
				if (Xf > Tf) {			/* zyxt */ \
					TH1_VERTEX(2, Y, ZI) \
					TH1_VERTEX(3, X, ZI+YI) \
					tfrac4 = Tf; \
				} \
				else { \
					if (Tf > Yf) {		/* ztyx */ \
						TH1_VERTEX(2, T, ZI) \
						TH1_VERTEX(3, Y, ZI+TI) \
					} \
					else {				/* zytx */ \
						TH1_VERTEX(2, Y, ZI) \
						TH1_VERTEX(3, T, ZI+YI) \
					} \
					tfrac4 = Xf; \
				} \
			} \
		} \
		else { \
			if (Yf > Tf) { \
				TH1_VERTEX(1, Y, 0) \
				if (Xf > Zf) { \
					if (Xf > Tf) {		/* yxtz */ \
						TH1_VERTEX(2, X, YI) \
						TH1_VERTEX(3, T, YI+XI) \
					} \
					else {				/* ytxz */ \
						TH1_VERTEX(2, T, YI) \
						TH1_VERTEX(3, X, YI+TI) \
					} \
					tfrac4 = Zf; \
				} \
				else {					/* ytzx */ \
					TH1_VERTEX(2, T, YI) \
					TH1_VERTEX(3, Z, YI+TI) \
					tfrac4 = Xf; \
				} \
			} \
			else { \
				TH1_VERTEX(1, T, 0) \
				if (Xf > Zf) {			/* tyxz */ \
					TH1_VERTEX(2, Y, TI) \
					TH1_VERTEX(3, X, TI+YI) \
					tfrac4 = Zf; \
				} \
				else { \
					if (Zf > Yf) {		/* tzyx */ \
						TH1_VERTEX(2, Z, TI) \
						TH1_VERTEX(3, Y, TI+ZI) \
					} \
					else {				/* tyzx */ \
						TH1_VERTEX(2, Y, TI) \
						TH1_VERTEX(3, Z, TI+YI) \
					} \
					tfrac4 = Xf; \
				} \
			} \
		} \
	}



#define TH1_4D_TETRAINTERP_D8(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert4Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert4)); \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)); \
	mullacc = tfrac4 * (tvert4Data - tvert3Data);					/* (tvert4 - tvert3) * f4 */ \
 \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)); \
	mullacc += (tfrac3 * (tvert3Data - tvert2Data));				/* (tvert3 - tvert2) * f3 */ \
 \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)); \
	mullacc += (tfrac2 * (tvert2Data - tvert1Data));				/* (tvert2 - tvert1) * f2 */ \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP)); \
	mullacc += (tfrac1 * (tvert1Data - tResult));					/* (tvert1 - tvert0) * f1 */ \
 \
	KCP_SHIFT_RIGHT_ROUND(mullacc, temp, 14) 			/* tvert0 + (mullacc) */ \
	tResult = (tResult << 2) + temp;

#define TH1_TEST_DELTA(delta, chan) \
	highBits = delta & EVAL_HIGH_MASK; \
	if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
		goto ExtendedPrecision##chan; \
	}

#define TH1_4D_TETRAINTERP_D16(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert4Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert4)); \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)); \
	delta3 = tvert4Data - tvert3Data; \
 \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)); \
	delta2 = tvert3Data - tvert2Data; \
 \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)); \
	delta1 = tvert2Data - tvert1Data; \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP)); \
	delta0 = tvert1Data - tResult; \
 \
	TH1_TEST_DELTA(delta3, chan) \
	mullacc = tfrac4 * delta3;								/* (tvert4 - tvert3) * t */ \
 \
	TH1_TEST_DELTA(delta2, chan) \
	mullacc += (tfrac3 * delta2);							/* (tvert3 - tvert2) * z */ \
 \
	TH1_TEST_DELTA(delta1, chan) \
	mullacc += (tfrac2 * delta1);							/* (tvert2 - tvert1) * y */ \
 \
	TH1_TEST_DELTA(delta0, chan) \
	mullacc += (tfrac1 * delta0);							/* (tvert1 - tvert0) * x */ \
 \
	KCP_SHIFT_RIGHT_ROUND(mullacc, temp, EVAL_D16_FRACBITS)	/* tvert0 + (mullacc) */ \
	tResult += temp; \
 \
 	goto evalDone##chan; \
 \
 ExtendedPrecision##chan: \
 	KCP_SHIFT_RIGHT(delta3, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta3 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH = tfrac4 * deltaHigh;						/* (tvert4 - tvert3) * z */ \
	mullacc = tfrac4 * delta3; \
 \
 	KCP_SHIFT_RIGHT(delta2, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta2 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += tfrac3 * deltaHigh;						/* (tvert3 - tvert2) * y */ \
	mullacc += tfrac3 * delta2; \
 \
 	KCP_SHIFT_RIGHT(delta1, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta1 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += tfrac2 * deltaHigh;						/* (tvert2 - tvert1) * y */ \
	mullacc += tfrac2 * delta1; \
 \
 	KCP_SHIFT_RIGHT(delta0, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta0 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH += tfrac1 * deltaHigh;						/* (tvert1 - tvert0) * x */ \
	mullacc += tfrac1 * delta0; \
 \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_EXTENDED_BITS)	/* discard unneeded low order bits */ \
 	mullacc += mullaccH;			/* add in high order bits */ \
 	mullacc += (((1 << (EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS)) -1) >> 1);	/* round */ \
 	KCP_SHIFT_RIGHT(mullacc, mullacc, EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS); \
 	tResult += mullacc; \
 \
 evalDone##chan:

#define TH1_4D_TETRAINTERP_AND_OLUT(chan, bits) \
	TH1_4D_TETRAINTERP_D##bits(chan)	/* tetrahedral interpolation for this channel */ \
	prevRes##chan = outLut##chan[tResult];

#define TH1_STORE_DATA(chan, bits) \
	*outp##chan = prevRes##chan;	/* write to buffer */ \
	outp##chan = (KpUInt##bits##_p)((KpUInt8_p)outp##chan + outStride##chan);	/* next location */


#define TH1_4D_INTERP_OLUT_STORE(chan, bits) \
	TH1_4D_TETRAINTERP_D##bits(chan)		/* tetrahedral interpolation for this channel */ \
	*outp##chan = outLut##chan[tResult];	/* write to destination buffer */ \
	outp##chan = (KpUInt##bits##_p)((KpUInt8_p)outp##chan + outStride##chan);	/* next location */


/**************************************************************
 * evalTh1i4o1 ---- 8 BIT
 *  Evaluation routine for evaluating 4 channel to 1 channel.
 */

void
	evalTh1i4o1d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D8
					
		TH1_4D_INTERP_OLUT_STORE(0, 8)	/* tetrahedral interpolation for channel 0 */
	}
}

/**************************************************************
 * evalTh1i4o1 ---- 16 bit
 *  Evaluation routine for evaluating 4 channel to 1 channel.
 */

void
	evalTh1i4o1d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D16

		TH1_4D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
	}
}

/**************************************************************
 * evalTh1i4o2 --- 8 bit
 **************************************************************/

void
	evalTh1i4o2d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D8
					
		TH1_4D_INTERP_OLUT_STORE(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 8)	/* and the remaining channels */
	}
}

/**************************************************************
 * evalTh1i4o2  ----  16 bit
 **************************************************************/

void
	evalTh1i4o2d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D16
				
		TH1_4D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
	}
}

/**************************************************************
 * evalTh1i4o3   -----   8 bit
 **************************************************************/
 
void
	evalTh1i4o3d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D8
				
		TH1_4D_INTERP_OLUT_STORE(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 8)	/* and the remaining channels */
		TH1_4D_INTERP_OLUT_STORE(2, 8)
	}
}


#if defined (KPMAC)

typedef union QDBuff_s {
	KpUInt8_t	cbuf[4];
	KpUInt32_t	lword;
} QDBuff_t;

/**************************************************************
 * evalTh1i4o3QD
 **************************************************************/
void
	evalTh1i4o3QD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	outpL;
QDBuff_t	QDoBuf;
KpUInt8_t	prevRes0, prevRes1, prevRes2;
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)

	if (outStride) {}

	outpL = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D8
				
		TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		TH1_4D_TETRAINTERP_AND_OLUT(2, 8)

		QDoBuf.lword = *outpL;				/* preserve alpha channel */

		QDoBuf.cbuf[1] = prevRes0;			/* use result from previous color evaluation */
		QDoBuf.cbuf[2] = prevRes1;
		QDoBuf.cbuf[3] = prevRes2;
		
		*outpL++ = QDoBuf.lword;
	}
}

#endif 	/* if defined KPMAC */


/**************************************************************
 * evalTh1i4o3   ---- 16 bit
 **************************************************************/
 
void
	evalTh1i4o3d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)
TH1_4D_OCHAN_VARS(16, 2)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)
	TH1_4D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D16
				
		TH1_4D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_4D_INTERP_OLUT_STORE(2, 16)
	}
}


/**************************************************************
 * evalTh1i4o4   ----  8 bit
 *  Evaluation routine for evaluating 4 channel to 4 channels.
 **************************************************************/
void
	evalTh1i4o4d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D8
				
		TH1_4D_INTERP_OLUT_STORE(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 8)	/* and the remaining channels */
		TH1_4D_INTERP_OLUT_STORE(2, 8)
		TH1_4D_INTERP_OLUT_STORE(3, 8)
	}
}


/**************************************************************
 * evalTh1i4oB32
 **************************************************************/

void
	evalTh1iB32oB32 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
KpUInt8_t	prevRes0, prevRes1, prevRes2, prevRes3;
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	if (inStride || outStride) {}

	for (i1 = n; i1 > 0; i1--) {
		srcData0 = *inp0++; 					/* get channel 0 input data */
		baseOffset = inLut0[(0*FUT_INPTBL_ENT) + srcData0].index; 	/* pass input data through input tables */
		Xf = inLut0[(0*FUT_INPTBL_ENT) + srcData0].frac;
		srcData1 = *inp0++; 					/* get channel 1 input data */
		baseOffset += inLut0[(1*FUT_INPTBL_ENT) + srcData1].index;
		Yf = inLut0[(1*FUT_INPTBL_ENT) + srcData1].frac;
		srcData2 = *inp0++; 					/* get channel 2 input data */
		baseOffset += inLut0[(2*FUT_INPTBL_ENT) + srcData2].index;
		Zf = inLut0[(2*FUT_INPTBL_ENT) + srcData2].frac;
		srcData3 = *inp0++; 					/* get channel 3 input data */
		baseOffset += inLut0[(3*FUT_INPTBL_ENT) + srcData3].index;
		Tf = inLut0[(3*FUT_INPTBL_ENT) + srcData3].frac;

		TH1_4D_CALC_TETRA
				
		TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
		TH1_4D_TETRAINTERP_AND_OLUT(3, 8)

		*outp0++ = prevRes0;
		*outp0++ = prevRes1;
		*outp0++ = prevRes2;
		*outp0++ = prevRes3;
	}
}

/**************************************************************
 * evalTh1i4oL32
 *  Evaluation routine for evaluating 4 channel to 4 channels,
 **************************************************************/
void
	evalTh1iL32oL32 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
KpUInt8_t	prevRes0, prevRes1, prevRes2, prevRes3;
TH1_4D_OCHAN_VARS(8, 0)
TH1_4D_OCHAN_VARS(8, 1)
TH1_4D_OCHAN_VARS(8, 2)
TH1_4D_OCHAN_VARS(8, 3)

	TH1_4D_INIT(8)
	TH1_4D_OCHAN_TABLES(8, 0)
	TH1_4D_OCHAN_TABLES(8, 1)
	TH1_4D_OCHAN_TABLES(8, 2)
	TH1_4D_OCHAN_TABLES(8, 3)

	if (inStride || outStride) {}
	
	for (i1 = n; i1 > 0; i1--) {
		srcData0 = inp3[3]; 					/* get channel 0 input data */
		baseOffset = inLut0[(0*FUT_INPTBL_ENT) + srcData0].index; 	/* pass input data through input tables */
		Xf = inLut0[(0*FUT_INPTBL_ENT) + srcData0].frac;
		srcData1 = inp3[2]; 					/* get channel 1 input data */
		baseOffset += inLut0[(1*FUT_INPTBL_ENT) + srcData1].index;
		Yf = inLut0[(1*FUT_INPTBL_ENT) + srcData1].frac;
		srcData2 = inp3[1]; 					/* get channel 2 input data */
		baseOffset += inLut0[(2*FUT_INPTBL_ENT) + srcData2].index;
		Zf = inLut0[(2*FUT_INPTBL_ENT) + srcData2].frac;
		srcData3 = inp3[0]; 					/* get channel 3 input data */
		baseOffset += inLut0[(3*FUT_INPTBL_ENT) + srcData3].index;
		Tf = inLut0[(3*FUT_INPTBL_ENT) + srcData3].frac;

		inp3 += 4;

		TH1_4D_CALC_TETRA
				
		TH1_4D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		TH1_4D_TETRAINTERP_AND_OLUT(2, 8)
		TH1_4D_TETRAINTERP_AND_OLUT(3, 8)

		outp3[0] = prevRes3;
		outp3[1] = prevRes2;
		outp3[2] = prevRes1;
		outp3[3] = prevRes0;

		outp3 += 4;
	}

}

/**************************************************************
 * evalTh1i4o4   ---- 16 bit
 *  Evaluation routine for evaluating 4 channel to 4 channels.
 **************************************************************/
void
	evalTh1i4o4d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)
TH1_4D_OCHAN_VARS(16, 2)
TH1_4D_OCHAN_VARS(16, 3)

	TH1_4D_INIT(16)
	TH1_4D_OCHAN_TABLES(16, 0)
	TH1_4D_OCHAN_TABLES(16, 1)
	TH1_4D_OCHAN_TABLES(16, 2)
	TH1_4D_OCHAN_TABLES(16, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_4D_GET_TETRA_D16
				
		TH1_4D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_4D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_4D_INTERP_OLUT_STORE(2, 16)
		TH1_4D_INTERP_OLUT_STORE(3, 16)
	}
}


/*********************************************************************************************/

/* evalTh1i3o3d16 with 16 bit in and out data, no output tables, no optimized tables */

#define EVAL_EXTENDED_BITS (8)
#define EVAL_REMAINDER_BITS (15)
#define ROUND_VALUE(bits) ((1 << (bits -1)) -1)

#define TH1_4D_OCHANX_TABLES(chan) \
 \
	if ((outputMask & FUT_BIT(chan)) != 0) { \
		TH1_4D_OCHAN_TABLES(16, chan) \
	} \
	gridBase##chan = (KpUInt8_p)gridBase[chan]; \

#define	INTERP_ITABLE_D16(chan, frac) \
	srcData##chan = *inp##chan; 			/* get input data */ \
	inp##chan = (KpUInt16_p)((KpUInt8_p)inp##chan + inStride##chan); \
 \
	sPosition = srcData##chan * iIndexFactor;	/* calculate the input table position */ \
	sPosition += (sPosition >> MF2_TBL_BITS);	/* compensate for fraction error */ \
	index = sPosition >> EVAL_D16_FRACBITS; \
 \
	tableData = (KpInt32_t) iTbl[chan][index];			/* pass input data through input tables */ \
 \
	if (index < (iTblEntries -1)) {	/* must interpolate */ \
		KpInt32_t	nextTableData, delta, highBits, deltaHigh, resultH; \
 \
		interpolant = (KpInt32_t) (sPosition & ((1 << EVAL_D16_FRACBITS) -1)); \
 \
		nextTableData = (KpInt32_t) iTbl[chan][index +1];	/* next table data */ \
 \
		delta = nextTableData - tableData; \
 \
		highBits = delta & EVAL_HIGH_MASK; \
 \
		if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
		 	KCP_SHIFT_RIGHT(delta, deltaHigh, EVAL_EXTENDED_BITS) \
			resultH = interpolant * deltaHigh; \
		 	delta &= ((1 << EVAL_EXTENDED_BITS) -1); \
			nextTableData = interpolant * delta; \
		 	nextTableData += ROUND_VALUE(EVAL_EXTENDED_BITS); \
	 		KCP_SHIFT_RIGHT(nextTableData, nextTableData, EVAL_EXTENDED_BITS)	/* discard unneeded low order bits */ \
		 	nextTableData += resultH;			/* add in high order bits */ \
 \
			nextTableData += ROUND_VALUE(EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS); \
			KCP_SHIFT_RIGHT(nextTableData, nextTableData, EVAL_D16_FRACBITS - EVAL_EXTENDED_BITS); \
		} \
		else { \
			nextTableData = interpolant * delta; \
		 	nextTableData += ROUND_VALUE(EVAL_D16_FRACBITS); \
			KCP_SHIFT_RIGHT(nextTableData, nextTableData, EVAL_D16_FRACBITS); \
		} \
 \
		tableData += nextTableData; \
	} \
 \
	sPosition = tableData * gIndexFactor[chan];	/* calculate the grid table position */ \
	sPosition += (sPosition >> MF2_TBL_BITS);	/* compensate for fraction error */ \
	index = sPosition >> EVAL_D16_FRACBITS; \
 \
 	dimSize = gridDim[chan]; \
 \
	if (index < dimSize -1) { \
		frac = sPosition & ((1 << EVAL_D16_FRACBITS) -1);	/* get grid interpolant */ \
	} \
	else { \
		frac = (1 << EVAL_D16_FRACBITS) -1; \
		index--; \
	}

#define TH1_4D_TETRAINTERP_AND_STORE_D16(chan) \
		if ((outputMask & FUT_BIT(chan)) != 0) { \
 \
			TH1_4D_TETRAINTERP_D16(chan)	/* tetrahedral interpolation for this channel */ \
 \
			*outp##chan = (KpUInt16_t)tResult; \
			outp##chan = (KpUInt16_p)((KpUInt8_p)outp##chan + outStride##chan); \
		}


/* needs offset increment table */
/* all itbls same # entries */
/* must not have otbls */

void
	evalTh1i4oXd16n (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
KpInt32_t		iTblEntries, futOutputs, outputMask, dimSize, gridDim[4];
KpInt32_t		interpolant, tableData, index;
KpUInt32_t		sPosition, iIndexFactor, gIndexFactor[4];
fut_p			fut;
mf2_tbldat_p	iTbl[4], gridBase[FUT_NOCHAN];
TH1_4D_OCHAN_VARS(16, 0)
TH1_4D_OCHAN_VARS(16, 1)
TH1_4D_OCHAN_VARS(16, 2)
TH1_4D_OCHAN_VARS(16, 3)

	TH1_4D_INIT(16)

	fut = PTTableP->dataP;

	for (i1 = 0, futOutputs = 0, outputMask = 0; i1 < FUT_NOCHAN; i1++) {
		if (fut->iomask.out & FUT_BIT(i1)) {			
			gridBase[futOutputs] = fut->chan[i1]->gtbl->refTbl;	/* make a list of gtbls */
			
			futOutputs++;
		}

		if (outp[i1].p16 != NULL) {	/* this output channel is being evaluated */
			outputMask |= FUT_BIT(i1);
		}
	}

	for (i1 = 1; i1 < 15; i1++) {
		a[i1] /= futOutputs;	/* non-interleaved grid tables */
	}
	tvert4 /= futOutputs;

	TH1_4D_OCHANX_TABLES(0)
	TH1_4D_OCHANX_TABLES(1)
	TH1_4D_OCHANX_TABLES(2)
	TH1_4D_OCHANX_TABLES(3)

	for (i1 = 0; i1 < 4; i1++) {
		fut_itbl_p	theITbl;

		theITbl = fut->itbl[i1];
		iTbl[i1] = theITbl->refTbl;			/* collect the input tables */

		iTblEntries = theITbl->refTblEntries;
		
		gridDim[i1] = theITbl->size;

		/* set up interpolation into grid table */
		gIndexFactor[i1] = (gridDim[i1] -1) << EVAL_D16_FRACBITS;	/* get extra precision */
																/* do not round - error is compensated in interpolation */
		gIndexFactor[i1] /= MF2_TBL_MAXVAL;
	}

	iIndexFactor = (iTblEntries -1) << EVAL_D16_FRACBITS;	/* get extra precision */
														/* do not round - error is compensated in interpolation */
	iIndexFactor /= MF2_TBL_MAXVAL;


	for (i1 = n; i1 > 0; i1--) {
		INTERP_ITABLE_D16(0, Xf);

		baseOffset = index;				/* cell starts with this index */

		INTERP_ITABLE_D16(1, Yf);

		baseOffset *= dimSize;			/* build cell index */
		baseOffset += index;			/* add in this index */

		INTERP_ITABLE_D16(2, Zf);

		baseOffset *= dimSize;			/* build cell index */
		baseOffset += index;			/* add in this index */

		INTERP_ITABLE_D16(3, Tf);

		baseOffset *= dimSize;			/* build cell index */
		baseOffset += index;			/* add in this index */
		
		baseOffset <<= 1;				/* make byte offset */

		TH1_4D_CALC_TETRA 	

		TH1_4D_TETRAINTERP_AND_STORE_D16(0)	/* tetrahedral interpolation for each channel */
		TH1_4D_TETRAINTERP_AND_STORE_D16(1)
		TH1_4D_TETRAINTERP_AND_STORE_D16(2)
		TH1_4D_TETRAINTERP_AND_STORE_D16(3)
	}
}
