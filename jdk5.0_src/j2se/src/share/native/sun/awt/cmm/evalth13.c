/*
 * @(#)evalth13.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
*   evalth13.c
*
*	tetrahedral interpolation evaluation functions
*
*	Author:			George Pawle
*
*	Creation Date:	20 Dec 96
*
*    COPYRIGHT (c) 1996-2003 Eastman Kodak Company
*    As an unpublished work pursuant to Title 17 of the United
*    States Code.  All rights reserved.
*/

#include "kcpcache.h"

#define EVAL_HIGH_MASK (0xfffffe00)
#define EVAL_EXTENDED_BITS (8)

#define TH1_3D_OCHAN_VARS(bits, chan) \
KpUInt8_p			gridBase##chan; \
KpInt32_t			outStride##chan; \
KpUInt##bits##_p	outLut##chan, outp##chan; \
KpUInt##bits##_t	prevRes##chan;

#define TH1_3D_INIT_VARS \
KpUInt8_p	gridBaseP, thisOutLut; \
evalILuti_p	inLut0, inLut1, inLut2; \
KpUInt32_t	srcData0, srcData1, srcData2, dataMask, a[8]; \
KpInt32_t	i1, oChan, tResult, tvert1, tvert2, tvert3, mullacc, temp, iLutEntries; \
KpInt32_t	inStride0 = inStride[0], inStride1 = inStride[1], inStride2 = inStride[2]; \
KpInt32_t	baseOffset, Xf, Yf, Zf, tfrac1, tfrac2, tfrac3; \
KpInt32_t	tvert1Data, tvert2Data, tvert3Data;

#define TH1_3D_INIT_INVARS_D8 \
KpUInt8_p	inp0 = inp[0].p8, inp1 = inp[1].p8, inp2 = inp[2].p8; \
KpUInt32_t	thisColor, prevColor = 0xffffffff;

#define TH1_3D_INIT_INVARS_D16 \
KpUInt16_p	inp0 = inp[0].p16, inp1 = inp[1].p16, inp2 = inp[2].p16;

#define TH1_3D_INIT_GRIDVARS_D8 

#define TH1_3D_INIT_GRIDVARS_D16 \
KpInt32_t	delta2, delta1, delta0, deltaHigh, mullaccH, highBits;

#define TH1_3D_INIT_OUTVARS(bits) \
KpInt32_t	outLutSize = EVAL_OLUT_ENTRIESD##bits * sizeof (KpUInt##bits##_t);

#define TH1_3D_INIT_IN_D8 \
	if (dataTypeI) {} \
	iLutEntries = 1 << 8; \
	inLut0 = PTTableP->etLuts[ET_I8].P;

#define TH1_3D_INIT_IN_D16 \
	if (dataTypeI == KCM_USHORT_12) { \
		iLutEntries = 1 << 12; \
		inLut0 = PTTableP->etLuts[ET_I12F1].P; \
	} \
	else { \
		iLutEntries = 1 << 16; \
		inLut0 = PTTableP->etLuts[ET_I16].P; \
	}

#define TH1_3D_INIT_OUT_D8 \
	if (dataTypeO) {} \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G12F1].P; \
	thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O8].P; \

#define TH1_3D_INIT_OUT_D16 \
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G16].P; \
	if (dataTypeO == KCM_USHORT_12) { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O12].P; \
	} \
	else { \
		thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O16].P; \
	}

#define TH1_3D_INIT_DATA(obits) \
	a[1] = PTTableP->etGOffsets[1];		/* copy grid offsets into locals */ \
	a[2] = PTTableP->etGOffsets[2]; \
	a[3] = PTTableP->etGOffsets[3]; \
	a[4] = PTTableP->etGOffsets[4]; \
	a[5] = PTTableP->etGOffsets[5]; \
	a[6] = PTTableP->etGOffsets[6]; \
	tvert3 = PTTableP->etGOffsets[7]; \
 \
	inLut1 = inLut0 + iLutEntries; \
	inLut2 = inLut1 + iLutEntries; \
	dataMask = iLutEntries -1;	/* set up data mask to prevent input table memory access violations */ \
	oChan = -1; \
	gridBaseP -= sizeof (mf2_tbldat_t); \
	thisOutLut -= outLutSize;

#define TH1_3D_INIT(bits) \
	TH1_3D_INIT_VARS \
	TH1_3D_INIT_INVARS_D##bits \
	TH1_3D_INIT_GRIDVARS_D##bits \
	TH1_3D_INIT_OUTVARS(bits) \
	TH1_3D_INIT_IN_D##bits \
	TH1_3D_INIT_OUT_D##bits \
	TH1_3D_INIT_DATA(bits)

#define TH1_3D_OCHAN_TABLES(obits, chan) \
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

#define TH1_3D_GET_DATA_D8 \
	srcData0 = *inp0; 								/* get channel 0 input data */ \
	inp0 += inStride0; \
	srcData1 = *inp1; 								/* get channel 1 input data */ \
	inp1 += inStride1; \
	srcData2 = *inp2; 								/* get channel 2 input data */ \
	inp2 += inStride2; \
 \
	thisColor = (srcData0 << 16) | (srcData1 << 8) | (srcData2);	/* calc this color   */ 

	
#define TH1_3D_FIND_TETRA_D8    \
	prevColor = thisColor;    \
\
	baseOffset = inLut0[(0*FUT_INPTBL_ENT) + srcData0].index; 	/* pass input data through input tables */ \
	Xf = inLut0[(0*FUT_INPTBL_ENT) + srcData0].frac; \
	baseOffset += inLut0[(1*FUT_INPTBL_ENT) + srcData1].index; \
	Yf = inLut0[(1*FUT_INPTBL_ENT) + srcData1].frac; \
	baseOffset += inLut0[(2*FUT_INPTBL_ENT) + srcData2].index; \
	Zf = inLut0[(2*FUT_INPTBL_ENT) + srcData2].frac; \
\
	TH1_3D_CALC_TETRA


#define TH1_3D_GET_TETRA_D16 \
	srcData0 = *inp0; 						/* get channel 0 input data */ \
	srcData0 &= dataMask; \
	inp0 = (KpUInt16_p)((KpUInt8_p)inp0 + inStride0); \
	srcData1 = *inp1; 						/* get channel 1 input data */ \
	srcData1 &= dataMask; \
	inp1 = (KpUInt16_p)((KpUInt8_p)inp1 + inStride1); \
	srcData2 = *inp2; 						/* get channel 2 input data */ \
	srcData2 &= dataMask; \
	inp2 = (KpUInt16_p)((KpUInt8_p)inp2 + inStride2); \
 \
	baseOffset = inLut0[srcData0].index;	/* pass input data through input tables */ \
	Xf = inLut0[srcData0].frac; \
	baseOffset += inLut1[srcData1].index; \
	Yf = inLut1[srcData1].frac; \
	baseOffset += inLut2[srcData2].index; \
	Zf = inLut2[srcData2].frac; \
\
	TH1_3D_CALC_TETRA 	


#define XI (1 << 2)
#define YI (1 << 1)
#define ZI (1 << 0)

#define TH1_VERTEX(vert, dim, inc) \
	tfrac##vert = dim##f; \
	tvert##vert = a[inc + dim##I];

#define	TH1_3D_CALC_TETRA   \
	/* find the tetrahedron in which the point is located */ \
	if (Xf > Yf) { \
		if (Yf > Zf) {	/* xyz */ \
			TH1_VERTEX(1, X, 0) \
			TH1_VERTEX(2, Y, XI) \
			tfrac3 = Zf; \
		} \
		else { \
			if (Xf > Zf) {	/* xzy */ \
				TH1_VERTEX(1, X, 0) \
				TH1_VERTEX(2, Z, XI) \
			} \
			else {			/* zxy */ \
				TH1_VERTEX(1, Z, 0) \
				TH1_VERTEX(2, X, ZI) \
			} \
			tfrac3 = Yf; \
		} \
	} \
	else { \
		if (Yf > Zf) { \
			TH1_VERTEX(1, Y, 0) \
			if (Xf > Zf) {	/* yxz */ \
				TH1_VERTEX(2, X, YI) \
				tfrac3 = Zf; \
			} \
			else {			/* yzx */ \
				TH1_VERTEX(2, Z, YI) \
				tfrac3 = Xf; \
			} \
		} \
		else {			/* zyx */ \
			TH1_VERTEX(1, Z, 0) \
			TH1_VERTEX(2, Y, ZI) \
			tfrac3 = Xf; \
		} \
	}


#define TH1_3D_TETRAINTERP_D8(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)); \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)); \
	mullacc = (tfrac3 * (tvert3Data - tvert2Data));					/* (tvert3 - tvert2) * f3 */ \
 \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)); \
	mullacc += (tfrac2 * (tvert2Data - tvert1Data));				/* (tvert2 - tvert1) * f2 */ \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP)); \
	mullacc += (tfrac1 * (tvert1Data - tResult));					/* (tvert1 - tvert0) * f1 */ \
 \
	KCP_SHIFT_RIGHT(mullacc, temp, 14)				/* tvert0 + (mullacc) */ \
	tResult = (tResult << 2) + temp;

#define TH1_TEST_DELTA(delta, chan) \
	highBits = delta & EVAL_HIGH_MASK; \
	if ((highBits != 0) && (highBits != EVAL_HIGH_MASK)) { \
		goto ExtendedPrecision##chan; \
	}

#define TH1_3D_TETRAINTERP_D16(chan) \
	gridBaseP = gridBase##chan + baseOffset; \
	tvert3Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert3)); \
	tvert2Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert2)); \
	delta2 = tvert3Data - tvert2Data; \
 \
	tvert1Data = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP + tvert1)); \
	delta1 = tvert2Data - tvert1Data; \
 \
	tResult = ((KpInt32_t)*(mf2_tbldat_p)(gridBaseP)); \
	delta0 = tvert1Data - tResult; \
 \
	TH1_TEST_DELTA(delta2, chan) \
	mullacc = (tfrac3 * delta2);							/* (tvert3 - tvert2) * z */ \
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
 	KCP_SHIFT_RIGHT(delta2, deltaHigh, EVAL_EXTENDED_BITS) \
 	delta2 &= ((1 << EVAL_EXTENDED_BITS) -1); \
	mullaccH = tfrac3 * deltaHigh;						/* (tvert3 - tvert2) * y */ \
	mullacc = tfrac3 * delta2; \
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

#define TH1_3D_TETRAINTERP_AND_OLUT(chan, bits) \
	TH1_3D_TETRAINTERP_D##bits(chan)	/* tetrahedral interpolation for this channel */ \
	prevRes##chan = outLut##chan[tResult];

#define TH1_STORE_DATA(chan, bits) \
	*outp##chan = prevRes##chan;	/* write to buffer */ \
	outp##chan = (KpUInt##bits##_p)((KpUInt8_p)outp##chan + outStride##chan);	/* next location */


#define TH1_3D_INTERP_OLUT_STORE(chan, bits) \
	TH1_3D_TETRAINTERP_AND_OLUT(chan, bits) \
	TH1_STORE_DATA(chan, bits)


/******************************************************************************/

void
	evalTh1i3o1d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
	}
}


/******************************************************************************/

void
	evalTh1i3o1d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(16, 0)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
	}
}


/******************************************************************************/

void
	evalTh1i3o2d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o2d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
	}
}


/******************************************************************************/

void
	evalTh1i3o3d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


/******************************************************************************/

void
	evalTh1iB24oB24 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}

	for (i1 = n; i1 > 0; i1--) {
		srcData0 = *inp0++; 					/* get channel 0 input data */
		srcData1 = *inp0++; 					/* get channel 1 input data */
		srcData2 = *inp0++; 					/* get channel 2 input data */

		thisColor = (srcData0 << 16) | (srcData1 << 8) | (srcData2);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		*outp0++ = prevRes0;	/* use result from previous color evaluation */
		*outp0++ = prevRes1;
		*outp0++ = prevRes2;
	}
}


/******************************************************************************/

void
	evalTh1iL24oL24 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}
	
	for (i1 = n; i1 > 0; i1--) {
		srcData2 = inp2[0]; 					/* get channel 2 input data */
		srcData1 = inp2[1]; 					/* get channel 1 input data */
		srcData0 = inp2[2]; 					/* get channel 0 input data */

		inp2 += 3;

		thisColor = (srcData0 << 16) | (srcData1 << 8) | (srcData2);	/* calc this color */
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}
		
		outp2[0] = prevRes2;		/* use result from previous color evaluation */
		outp2[1] = prevRes1;
		outp2[2] = prevRes0;

		outp2 += 3;
	}
}


#if defined (KPMAC)
/**************************** Quick Draw Formats **********************************/

void
	evalTh1iQDoQD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	inpqd, outpqd;
KpUInt32_t	alphaData;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (inStride || outStride) {}
	
	inpqd = (KpUInt32_p)(inp[0].p8 -1);
	outpqd = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		thisColor = *inpqd++;
		alphaData = thisColor & 0xff000000;		/* get alpha channel input data */
		srcData0 = (thisColor >> 16) & 0xff;		/* get channel 0 input data */
		srcData1 = (thisColor >> 8) & 0xff;		/* get channel 1 input data */
		srcData2 = (thisColor) & 0xff;				/* get channel 2 input data */

		thisColor &= 0xffffff;					/* clear off alpha channel */

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		alphaData |= prevRes0 << 16;
		alphaData |= prevRes1 << 8;
		alphaData |= prevRes2;
		*outpqd++ = alphaData;
	}
}


/******************************************************************************/

void
	evalTh1iQDo3 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	inpqd;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
		
	if (inStride) {}
	
	inpqd = (KpUInt32_p)(inp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		thisColor = *inpqd++;
		srcData0 = (thisColor >> 16) & 0xff;		/* get channel 0 input data */
		srcData1 = (thisColor >> 8) & 0xff;		/* get channel 1 input data */
		srcData2 = (thisColor) & 0xff;				/* get channel 2 input data */
		
		thisColor &= 0xffffff;					/* clear off alpha channel */

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3oQD (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpUInt32_p	outpqd;
KpUInt32_t	qdData;

TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)

	if (outStride) {}
	
	outpqd = (KpUInt32_p)(outp[0].p8 -1);

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8

		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
		}

		qdData  = prevRes0 << 16;
		qdData |= prevRes1 << 8;
		qdData |= prevRes2;
		*outpqd++ = qdData;
	}
}

#endif 	/* KPMAC */


/*********************************************************************************************/

void
	evalTh1i3o3d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
	}
}


/*********************************************************************************************/

/* evalTh1i3o3d16 with 16 bit in and out data, no output tables, no optimized tables */

#define EVAL_EXTENDED_BITS (8)
#define EVAL_REMAINDER_BITS (15)
#define ROUND_VALUE(bits) ((1 << (bits -1)) -1)

#define TH1_3D_OCHANX_TABLES(chan) \
 \
	if ((outputMask & FUT_BIT(chan)) != 0) { \
		TH1_3D_OCHAN_TABLES(16, chan) \
	} \
	gridBase##chan = (KpUInt8_p)gridBase[chan]; \
	prevRes##chan = 0;

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

#define TH1_3D_TETRAINTERP_AND_STORE_D16(chan) \
		if ((outputMask & FUT_BIT(chan)) != 0) { \
 \
			TH1_3D_TETRAINTERP_D16(chan)	/* tetrahedral interpolation for channel 1 */ \
 \
			*outp##chan = (KpUInt16_t)tResult; \
			outp##chan = (KpUInt16_p)((KpUInt8_p)outp##chan + outStride##chan); \
		}


/* needs offset increment table */
/* all itbls same # entries */
/* must not have otbls */

void
	evalTh1i3oXd16n (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
KpInt32_t		iTblEntries, futOutputs, outputMask, dimSize, gridDim[3];
KpInt32_t		interpolant, tableData, index;
KpUInt32_t		sPosition, iIndexFactor, gIndexFactor[3];
fut_p			fut;
mf2_tbldat_p	iTbl[3], gridBase[FUT_NOCHAN];
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)
TH1_3D_OCHAN_VARS(16, 6)
TH1_3D_OCHAN_VARS(16, 7)

	TH1_3D_INIT(16)

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

	for (i1 = 1; i1 < 7; i1++) {
		a[i1] /= futOutputs;	/* non-interleaved grid tables */
	}
	tvert3 /= futOutputs;

	TH1_3D_OCHANX_TABLES(0)
	TH1_3D_OCHANX_TABLES(1)
	TH1_3D_OCHANX_TABLES(2)
	TH1_3D_OCHANX_TABLES(3)
	TH1_3D_OCHANX_TABLES(4)
	TH1_3D_OCHANX_TABLES(5)
	TH1_3D_OCHANX_TABLES(6)
	TH1_3D_OCHANX_TABLES(7)

	for (i1 = 0; i1 < 3; i1++) {
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
		
		baseOffset <<= 1;				/* make byte offset */

		TH1_3D_CALC_TETRA 	

		TH1_3D_TETRAINTERP_AND_STORE_D16(0)	/* tetrahedral interpolation for each channel */
		TH1_3D_TETRAINTERP_AND_STORE_D16(1)
		TH1_3D_TETRAINTERP_AND_STORE_D16(2)
		TH1_3D_TETRAINTERP_AND_STORE_D16(3)
		TH1_3D_TETRAINTERP_AND_STORE_D16(4)
		TH1_3D_TETRAINTERP_AND_STORE_D16(5)
		TH1_3D_TETRAINTERP_AND_STORE_D16(6)
		TH1_3D_TETRAINTERP_AND_STORE_D16(7)
	}
}


/* Tetrahedral Interpolation evaluation of 3-channel 8-bit to 3-channel 12/16-bit data */
void
	evalTh1i3o3d8to16 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)

	TH1_3D_INIT_VARS
	TH1_3D_INIT_INVARS_D8
	TH1_3D_INIT_GRIDVARS_D16
	TH1_3D_INIT_OUTVARS(16)
	TH1_3D_INIT_IN_D8
	TH1_3D_INIT_OUT_D16
	TH1_3D_INIT_DATA(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 16)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 16)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 16)
		}

		TH1_STORE_DATA(0, 16);		/* store the data */
		TH1_STORE_DATA(1, 16);
		TH1_STORE_DATA(2, 16);
	}
}

/* Tetrahedral Interpolation evaluation of 3-channel 12/16-bit to 3-channel 8-bit data */
void
	evalTh1i3o3d16to8 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)

	if(((inStride[0] == 6) && (inStride[1] == 6) && (inStride[2] == 6)) &&
		(dataTypeI == KCM_USHORT_12)									&&
		((outStride[0] == 3) && (outStride[1] == 3) && (outStride[2] == 3)) &&
		(dataTypeO == KCM_UBYTE) &&
		(PTTableP->etGOffsets[1] == 6) && (PTTableP->etGOffsets[2] == 192)
		&& (PTTableP->etGOffsets[4] == 6144))
		evalTh1i3o3d16to8QS(inp, inStride, dataTypeI, outp, outStride, dataTypeO, n, PTTableP); 
	else
	{
		TH1_3D_INIT_VARS
		TH1_3D_INIT_INVARS_D16
		TH1_3D_INIT_GRIDVARS_D8
		TH1_3D_INIT_OUTVARS(8)
		TH1_3D_INIT_IN_D16
		TH1_3D_INIT_OUT_D8
		TH1_3D_INIT_DATA(8)
		TH1_3D_OCHAN_TABLES(8, 0)
		TH1_3D_OCHAN_TABLES(8, 1)
		TH1_3D_OCHAN_TABLES(8, 2)

		for (i1 = n; i1 > 0; i1--) {
			TH1_3D_GET_TETRA_D16
			
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)

			TH1_STORE_DATA(0, 8);		/* store the data */
			TH1_STORE_DATA(1, 8);
			TH1_STORE_DATA(2, 8);
		}
	}
}


/* Tetrahedral Interpolation evaluation of 3-channel 12/16-bit to 3-channel 8-bit data */
/*  This method is for Quick Silver system only */
#define QS_A001		6
#define QS_A010		192
#define QS_A011		198
#define QS_A100		6144
#define QS_A101		6150
#define QS_A110		6336
#define QS_TVERT3	6342
#define QS_OUTLUTSIZE	16384
#define QS_2XOUTLUTSIZE	32768
#define QS_INPUTSTRIDE	3	/* pointer */
#define	QS_OUTPUTSTRIDE	3	/* bytes */
void
 evalTh1i3o3d16to8QS ( imagePtr_p inp,
      KpInt32_p inStride,
      KpUInt32_t dataTypeI,
      imagePtr_p outp,
      KpInt32_p outStride,
      KpUInt32_t dataTypeO,
      KpInt32_t n,
      PTTable_p PTTableP)
{
	/* TH1_3D_OCHAN_VARS(8, 0) */
	KpUInt8_p gridBase0; 
	/* KpInt32_t outStride0; */
	KpUInt8_p outLut0, outp0; 
	KpUInt8_t prevRes0;

	/* TH1_3D_OCHAN_VARS(8, 1) */
	KpUInt8_p gridBase1; 
	/* KpInt32_t outStride1; */
	KpUInt8_p outLut1, outp1; 
	KpUInt8_t prevRes1;

	/* TH1_3D_OCHAN_VARS(8, 2) */
	KpUInt8_p gridBase2; 
	/* KpInt32_t outStride2; */
	KpUInt8_p outLut2, outp2; 
	KpUInt8_t prevRes2;

	/* TH1_3D_INIT_VARS  */
	KpUInt8_p gridBaseP, thisOutLut; 
	evalILuti_p inLut0, inLut1, inLut2; 
	KpUInt32_t data0, data1, data2; 
	KpInt32_t i1, tvert1, tvert2, iLutEntries; 
	KpInt32_t baseOffset, Xf, Yf, Zf, xf, yf, zf; 

	/* TH1_3D_INIT_INVARS_D16 */
	KpUInt16_p inp0 = inp[0].p16, inp1 = inp[1].p16, inp2 = inp[2].p16; 

	/* TH1_3D_INIT_IN_D16 */
	iLutEntries = 1 << 12; 
	inLut0 = PTTableP->etLuts[ ET_I12F1].P; 
	
	/* TH1_3D_INIT_OUT_D8 */
	gridBaseP = (KpUInt8_p)PTTableP->etLuts[ET_G12F1].P; 
	thisOutLut = (KpUInt8_p)PTTableP->etLuts[ET_O8].P;
	
	inLut1 = inLut0 + iLutEntries; 
	inLut2 = inLut1 + iLutEntries; 

	/* TH1_3D_OCHAN_TABLES(8, 0) */
	outp0 = outp[0].p8; 
	gridBase0 = gridBaseP; 
	outLut0 = thisOutLut; /* (KpUInt8_p)thisOutLut; */

	/* TH1_3D_OCHAN_TABLES(8, 1) */
	outp1 = outp[1].p8; 
	gridBase1 = gridBaseP + 2; 
	outLut1 = thisOutLut + QS_OUTLUTSIZE; /* (KpUInt8_p)thisOutLut; */

	/* TH1_3D_OCHAN_TABLES(8, 2) */
	outp2 = outp[2].p8; 
	gridBase2 = gridBaseP + 4; 
	outLut2 = thisOutLut + QS_2XOUTLUTSIZE; /* (KpUInt8_p)thisOutLut; */

	for (i1 = 0; i1 < n; i1++) {
		/* TH1_3D_GET_TETRA_D16 */
		data0 = *(inp0 + i1*QS_INPUTSTRIDE);				/* get channel 0 input data */
		data1 = *(inp1 + i1*QS_INPUTSTRIDE);				/* get channel 1 input data */
		data2 = *(inp2 + i1*QS_INPUTSTRIDE);				/* get channel 2 input data */
		
		baseOffset = inLut0[data0].index;		/* pass input data through input tables */
		Xf = inLut0[data0].frac; 
		baseOffset += inLut1[data1].index; 
		Yf = inLut1[data1].frac; 
		baseOffset += inLut2[data2].index; 
		Zf = inLut2[data2].frac; 
			
		/* find the tetrahedron in which the point is located */
		if (Xf > Yf) { 
			if (Yf > Zf) {  /* AHEG	*/
				xf = Xf;	/* unchanged */
				yf = Yf; 
				zf = Zf; 
				tvert2 = QS_A110; /* a110; */
				tvert1 = QS_A100; /* a100; */

			} 
			else { 
				zf = Yf;	/* y into z */
				tvert2 = QS_A101; /* a101; */
				if (Xf > Zf) {  /* AHEF */
					xf = Xf;	/* x does not change */
					yf = Zf;	/* z into y */
					tvert1 = QS_A100; /* a100; */
				} 
				else {		/* AHBF */
					xf = Zf;		/* z into x */
					yf = Xf;		/* x into y */
					tvert1 = QS_A001; /* a001; */
				} 
			} 
		} 
		else { 
			if (Yf > Zf) { 
				xf = Yf;		/* y into x */
				tvert1 = QS_A010; /* a010; */
				if (Xf > Zf) {  /* AHCG */
					yf = Xf;	/* x into y */
					zf = Zf;	/* z into z */
					tvert2 = QS_A110; /* a110; */
				} 
				else {		/* AHCD	*/
					yf = Zf;	/* z into y	*/
					zf = Xf;	/* z into z	*/
					tvert2 = QS_A011; /* a011; */
				} 
			} 
			else {			/*	AHDB	*/
				xf = Zf;	/*	z into x	*/
				yf = Yf;	/*	y into y	*/
				zf = Xf;	/*	x into z	*/
				tvert2 = QS_A011; /* a011; */
				tvert1 = QS_A001; /* a001; */
			} 
		}

		{
			register long	tResult;
			register long	tvertData;
			register long	mullacc;

			/* TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	 tetrahedral interpolation for channel 0 */
			tResult = *(mf2_tbldat_p)(gridBase0 + baseOffset + tvert2); /* & (mask); */
			tvertData = (*(mf2_tbldat_p)(gridBase0 + baseOffset + QS_TVERT3/*tvert3*/)); /* & (mask); */
			mullacc = zf * (tvertData - tResult);	/* (tvert3 - tvert2)*z	*/
			tvertData = (*(mf2_tbldat_p)(gridBase0 + baseOffset + tvert1)); /* & (mask); */
			mullacc += (yf * (tResult - tvertData));	/*	(tvert2 - tvert1)*y	*/
			tResult = (*(mf2_tbldat_p)(gridBase0 + baseOffset)); /* & (mask); */
			mullacc += (xf * (tvertData - tResult));	/* (tvert1 - A)*x	*/ 
			prevRes0 = outLut0[(tResult << 2) + (mullacc >> 14)]; 


			/* TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	 and the remaining channels */
			tResult = (*(mf2_tbldat_p)(gridBase1 + baseOffset + tvert2)); /* & (mask); */
			tvertData = (*(mf2_tbldat_p)(gridBase1 + baseOffset + QS_TVERT3/*tvert3*/)); /* & (mask); */
			mullacc = zf * (tvertData - tResult);	/* (tvert3 - tvert2)*z	*/
			tvertData = (*(mf2_tbldat_p)(gridBase1 + baseOffset + tvert1)); /* & (mask); */
			mullacc += (yf * (tResult - tvertData)); /*	(tvert2 - tvert1)*y	*/
			tResult = (*(mf2_tbldat_p)(gridBase1 + baseOffset)); /* & (mask); */
			mullacc += (xf * (tvertData - tResult)); /* (tvert1 - A)*x	*/ 
			prevRes1 = outLut1[(tResult << 2) +  + (mullacc >> 14)]; 
			

			/* TH1_3D_TETRAINTERP_AND_OLUT(2, 8) */
			tResult = (*(mf2_tbldat_p)(gridBase2 + baseOffset + tvert2)); /* & (mask); */
			tvertData = (*(mf2_tbldat_p)(gridBase2 + baseOffset + QS_TVERT3/*tvert3*/)); /* & (mask); */
			mullacc = zf * (tvertData - tResult);	/* (tvert3 - tvert2)*z	*/
			tvertData = (*(mf2_tbldat_p)(gridBase2 + baseOffset + tvert1)); /* & (mask); */
			mullacc += (yf * (tResult - tvertData));	/*	(tvert2 - tvert1)*y	*/
			tResult = (*(mf2_tbldat_p)(gridBase2 + baseOffset)); /* & (mask); */
			mullacc += (xf * (tvertData - tResult));	/* (tvert1 - A)*x	*/ 
			prevRes2 = outLut2[(tResult << 2) + (mullacc >> 14)];
			
		}

		/* TH1_STORE_DATA(0, 8);		 store the data */
		*outp0 = prevRes0;	/* write to buffer	*/
		outp0 += QS_OUTPUTSTRIDE;  

		/* TH1_STORE_DATA(1, 8); */
		*outp1 = prevRes1; /* write to buffer	*/
		outp1 += QS_OUTPUTSTRIDE;
		
		/* TH1_STORE_DATA(2, 8); */
		*outp2 = prevRes2;	/* write to buffer	*/
		outp2 += QS_OUTPUTSTRIDE;

	}
}


/******************************************************************************/

void
	evalTh1i3o4d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o4d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
		TH1_3D_INTERP_OLUT_STORE(3, 16)
	}
}


/******************************************************************************/

void
	evalTh1i3o5d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o5d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
		TH1_3D_INTERP_OLUT_STORE(3, 16)
		TH1_3D_INTERP_OLUT_STORE(4, 16)
	}
}


/******************************************************************************/

void
	evalTh1i3o6d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o6d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
		TH1_3D_INTERP_OLUT_STORE(3, 16)
		TH1_3D_INTERP_OLUT_STORE(4, 16)
		TH1_3D_INTERP_OLUT_STORE(5, 16)
	}
}

/******************************************************************************/

void
	evalTh1i3o7d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)
TH1_3D_OCHAN_VARS(8, 6)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)
	TH1_3D_OCHAN_TABLES(8, 6)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
		TH1_STORE_DATA(6, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o7d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)
TH1_3D_OCHAN_VARS(16, 6)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)
	TH1_3D_OCHAN_TABLES(16, 6)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
		TH1_3D_INTERP_OLUT_STORE(3, 16)
		TH1_3D_INTERP_OLUT_STORE(4, 16)
		TH1_3D_INTERP_OLUT_STORE(5, 16)
		TH1_3D_INTERP_OLUT_STORE(6, 16)
	}
}


/******************************************************************************/

void
	evalTh1i3o8d8 (	imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
TH1_3D_OCHAN_VARS(8, 0)
TH1_3D_OCHAN_VARS(8, 1)
TH1_3D_OCHAN_VARS(8, 2)
TH1_3D_OCHAN_VARS(8, 3)
TH1_3D_OCHAN_VARS(8, 4)
TH1_3D_OCHAN_VARS(8, 5)
TH1_3D_OCHAN_VARS(8, 6)
TH1_3D_OCHAN_VARS(8, 7)

	TH1_3D_INIT(8)
	TH1_3D_OCHAN_TABLES(8, 0)
	TH1_3D_OCHAN_TABLES(8, 1)
	TH1_3D_OCHAN_TABLES(8, 2)
	TH1_3D_OCHAN_TABLES(8, 3)
	TH1_3D_OCHAN_TABLES(8, 4)
	TH1_3D_OCHAN_TABLES(8, 5)
	TH1_3D_OCHAN_TABLES(8, 6)
	TH1_3D_OCHAN_TABLES(8, 7)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_DATA_D8
		
		if (thisColor != prevColor) {

			TH1_3D_FIND_TETRA_D8
					
			TH1_3D_TETRAINTERP_AND_OLUT(0, 8)	/* tetrahedral interpolation for channel 0 */
			TH1_3D_TETRAINTERP_AND_OLUT(1, 8)	/* and the remaining channels */
			TH1_3D_TETRAINTERP_AND_OLUT(2, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(3, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(4, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(5, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(6, 8)
			TH1_3D_TETRAINTERP_AND_OLUT(7, 8)
		}

		TH1_STORE_DATA(0, 8);		/* store the data */
		TH1_STORE_DATA(1, 8);
		TH1_STORE_DATA(2, 8);
		TH1_STORE_DATA(3, 8);
		TH1_STORE_DATA(4, 8);
		TH1_STORE_DATA(5, 8);
		TH1_STORE_DATA(6, 8);
		TH1_STORE_DATA(7, 8);
	}
}


/******************************************************************************/

void
	evalTh1i3o8d16 (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)

{
TH1_3D_OCHAN_VARS(16, 0)
TH1_3D_OCHAN_VARS(16, 1)
TH1_3D_OCHAN_VARS(16, 2)
TH1_3D_OCHAN_VARS(16, 3)
TH1_3D_OCHAN_VARS(16, 4)
TH1_3D_OCHAN_VARS(16, 5)
TH1_3D_OCHAN_VARS(16, 6)
TH1_3D_OCHAN_VARS(16, 7)

	TH1_3D_INIT(16)
	TH1_3D_OCHAN_TABLES(16, 0)
	TH1_3D_OCHAN_TABLES(16, 1)
	TH1_3D_OCHAN_TABLES(16, 2)
	TH1_3D_OCHAN_TABLES(16, 3)
	TH1_3D_OCHAN_TABLES(16, 4)
	TH1_3D_OCHAN_TABLES(16, 5)
	TH1_3D_OCHAN_TABLES(16, 6)
	TH1_3D_OCHAN_TABLES(16, 7)

	for (i1 = n; i1 > 0; i1--) {
		TH1_3D_GET_TETRA_D16
				
		TH1_3D_INTERP_OLUT_STORE(0, 16)	/* tetrahedral interpolation for channel 0 */
		TH1_3D_INTERP_OLUT_STORE(1, 16)	/* and the remaining channels */
		TH1_3D_INTERP_OLUT_STORE(2, 16)
		TH1_3D_INTERP_OLUT_STORE(3, 16)
		TH1_3D_INTERP_OLUT_STORE(4, 16)
		TH1_3D_INTERP_OLUT_STORE(5, 16)
		TH1_3D_INTERP_OLUT_STORE(6, 16)
		TH1_3D_INTERP_OLUT_STORE(7, 16)
	}
}


#if defined KCP_PENTIUM

/* address definitions for pixel cube corner access; use these for SSE math, too */
#define P000_SSE [edi]						/* base pointer */
#define P001_SSE [edi+SIZEOF_M128]
#define P010_SSE [edi+eax]					/* P010 offset in eax */
#define P011_SSE [edi+eax+SIZEOF_M128]	/* P010 + SIZEOF_M128 */
#define P100_SSE [edi+ecx]					/* P100 offset in ecx */
#define P101_SSE [edi+ecx+SIZEOF_M128]
#define P110_SSE [edi+esi]					/* P110 offset in esi */
#define P111_SSE [edi+esi+SIZEOF_M128]

/* definition for jump table for MMX, etc. code */
typedef void (*pFunction) (void);
	
/* tetrahedral interpolation using SSE */

void evalTh1i3o3D12ISSE (imagePtr_p	inp,
					KpInt32_p	inStride,
					KpUInt32_t	dataTypeI,
					imagePtr_p	outp,
					KpInt32_p	outStride,
					KpUInt32_t	dataTypeO,
					KpInt32_t	n,
					PTTable_p	PTTableP)
{
KpInt32_t	i1, srcOffset, srcPelStride, dstOffset, dstPelStride;
KpUInt8_p	pSrcPixel, pDstPixel;
evalILutSSE_p	m_pInputLut;
KpUInt16_p	m_pOutputLut;
static	pFunction pJmpTable[8] = {0,0,0,0,0,0,0,0};
KpInt32_t   m_iP001, m_iP010, m_iP011, m_iP100, m_iP101, m_iP110, m_iP111;

	m_pInputLut = (evalILutSSE_p)PTTableP->etLuts[ET_I12F3].P;
	m_pOutputLut = PTTableP->etLuts[ET_O12].P;

	m_iP001 = PTTableP->etGOffsets[1] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one up				(x+0, y+0, z+1) */
	m_iP010 = PTTableP->etGOffsets[2] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one back				(x+0, y+1, z+0) */
	m_iP011 = PTTableP->etGOffsets[3] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one up and back 		(x+0, y+1, z+1) */
	m_iP100 = PTTableP->etGOffsets[4] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right			(x+1, y+0, z+0) */
	m_iP101 = PTTableP->etGOffsets[5] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right and up		(x+1, y+0, z+1) */
	m_iP110 = PTTableP->etGOffsets[6] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right and back	(x+1, y+1, z+0) */
	m_iP111 = PTTableP->etGOffsets[7] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right, back, up	(x+1, y+1, z+1) */

	/* set up counter & pointer info */
	i1 = n;
	pSrcPixel = inp[0].p8;
	srcOffset = inp[1].p8 - pSrcPixel;
	srcPelStride = inStride[0];
	pDstPixel = outp[0].p8;
	dstOffset = outp[1].p8 - pDstPixel;
	dstPelStride = outStride[0];

	_asm
	{
		push		ebx				/* save this guy - compiler bug for now */

		/* load up jump table
		 * AHEG:	Y <  X; Z <  X; Z <  Y		1  1  1
		 * AHEF:	Y <  X; Z <  X; Z >= Y		1  1  0
		 *			Y <  X; Z >= X; Z <  Y		1  0  1   - not possible
		 * AHBF:	Y <  X; Z >= X; Z >= Y		1  0  0
		 * AHCD:	Y >= X; Z >= X; Z <  Y		0  0  1
		 * AHBD:	Y >= X; Z >= X; Z >= Y		0  0  0
		 *			Y >= X; Z <  X; Z >= Y      0  1  0   - not possible
		 * AHCG:	Y >= X; Z <  X; Z <  Y		0  1  1
		 */
		cmp			dword ptr [pJmpTable], 0					/* skip if already initialized */
		jnz			sse_3dlut_10

		mov			eax, offset sse_AHEG

		mov			[pJmpTable + 7*4], eax						/* AHEG: 1  1  1 */
		mov			ebx, offset sse_AHEF

		mov			[pJmpTable + 6*4], ebx						/* AHEF: 1  1  0 */
		mov			ecx, offset sse_AHBF
														
		mov			[pJmpTable + 4*4], ecx						/* AHBF: 1  0  0 */
		mov			eax, offset sse_AHCD
															
		mov			[pJmpTable + 1*4], eax						/* AHCD: 0  0  1 */
		mov			ebx, offset sse_AHBD

		mov			[pJmpTable + 0*4], ebx						/* AHBD: 0  0  0 */
		mov			ecx, offset sse_AHCG

		mov			[pJmpTable + 3*4], ecx						/* AHCG: 0  1  1 */
		mov			eax, offset sse_Invalid

		mov			[pJmpTable + 5*4], eax						/* Bad:  1  0  1 */

		mov			[pJmpTable + 2*4], eax						/* Bad:  0  1  0 */

sse_3dlut_10:

sse_3dlut_40:													/*  do { */
		/* use input shaping LUT; base address of grid table is part of red (1st chan.) index.
		 * P000 = (Pixel*)(
		 *		m_pInputLut [pSrcPixel->Red                    ].index +  /* x
		 *		m_pInputLut [pSrcPixel->Green + IN_LUT_SIZE_D12    ].index +  /* y
		 *		m_pInputLut [pSrcPixel->Blue  + (2*IN_LUT_SIZE_D12)].index);  /* z
		 * Pa = P000; Pb = P001; Pc = P010; Pd = P011
		 * Pe = P100; Pf = P101; Pg = P110; Ph = P111

		 * get fractional parts of values for interpolation
		 * iXFrac = m_pInputLut [pSrcPixel->Red                    ].fraction;
		 * iYFrac = m_pInputLut [pSrcPixel->Green + IN_LUT_SIZE_D12    ].fraction;
		 * iZFrac = m_pInputLut [pSrcPixel->Blue  + (2*IN_LUT_SIZE_D12)].fraction;
		 *
		 * 2 issues when loading up fractions:
		 * - one set of XMM registers needs to be: 4 - x fraction, 4 - y fraction, 4 - z fraction
		 *   used for multiplying pixels by fractions in calculation
		 * - one set of XMM registers used for comparison: YZZ values and XXY values
		 *   compare YYZZ to YXXY values to get 3 nonzero flags represent compare results
		 *   move flags to register for use in indexed jump 
		 *   (Y compared to Y gives 0 for less than compare, so the largest bit is always 0)
		 */
		mov			esi, pSrcPixel								/* esi = pSrcPixel */
		mov			ecx, srcOffset								/* ecx = source offset */
		mov			edx, m_pInputLut							/* edx = table base */

		movzx		eax, word ptr [esi]							/* get red pixel value */
		movzx		ebx, word ptr [esi+ecx]						/* get green pixel value */
		and			eax, 0fffh									/* restrict data to 12 bits */

		mov			edi, [edx + 8*eax]							/* get starting index */
		and			ebx, 0fffh
		movss		xmm0,[edx + 8*eax + 4]						/* xmm0 = fXFrac */

		add			edi, [edx + 8*ebx + ((1 << 12)*IN_LUT_ENTRY_SIZE_SSE)]
																/* add in green index */
		movzx		ecx, word ptr [esi+2*ecx]					/* get blue pixel value */

		movss		xmm1,[edx + 8*ebx + ((1 << 12)*IN_LUT_ENTRY_SIZE_SSE) + 4]
		and			ecx, 0fffh
																/* xmm1 = fYFrac */
		movaps		xmm3, xmm0									/* xmm3 = iXFrac */
		add			esi, srcPelStride							/* next source pixel */

		mov			pSrcPixel, esi

		add			edi, [edx + 8*ecx + (2*(1 << 12)*IN_LUT_ENTRY_SIZE_SSE)]
		movss		xmm2,[edx + 8*ecx + (2*(1 << 12)*IN_LUT_ENTRY_SIZE_SSE) + 4]
																/* xmm2 = fZFrac
		/* SSE access definitions; shown here for convenient documentation; defined previously.
		 * #define P000_SSE [edi]						/* base pointer
		 * #define P001_SSE [edi+SIZEOF_M128]
		 * #define P010_SSE [edi+eax]					/* P010 offset in eax
		 * #define P011_SSE [edi+eax+SIZEOF_M128]		/* P010 + SIZEOF_M128
		 * #define P100_SSE [edi+ecx]					/* P100 offset in ecx
		 * #define P101_SSE [edi+ecx+SIZEOF_M128]
		 * #define P110_SSE [edi+esi]					/* P110 offset in esi
		 * #define P111_SSE [edi+esi+SIZEOF_M128]
		 */

		/* prepare fractions for multiplying and comparing */
		shufps		xmm3, xmm1, 0								/* xmm3 = YYXX */
		mov			eax, m_iP010								/* eax  = pixel P010 offset */

		shufps		xmm1, xmm1, 0								/* xmm1 = fYFrac,fYFrac,fYFrac,fYFrac (replicated) */
		movaps		xmm5, xmm2									/* xmm5 = iZFrac */

		shufps		xmm5, xmm1, 0								/* xmm5 = YYZZ */
		mov			ecx, m_iP100								/* ecx  = pixel P100 offset */

		shufps		xmm0, xmm0, 0								/* xmm0 = fXfrac,fXFrac,fXFrac,fXFrac (replicated) */
		mov			esi, m_iP110								/* esi  = pixel P110 offset */

		shufps		xmm3, xmm3, 0xd3							/* xmm3 = YXXY (0xd3 = 0b11 01 00 11 */
		movaps		xmm6, P000_SSE								/* xmm6  = P000->RGB */

		shufps		xmm2, xmm2, 0								/* xmm2 = fZFrac,fZFrac,fZFrac,fZFrac (replicated) */

		cmpltps		xmm5, xmm3									/* xmm5 = truth flags for xmm5 < xmm3 */
		movaps		xmm7, P111_SSE								/* xmm7  = P111->RGB */

		movmskps	ebx, xmm5									/* ebx  = hi bits of flags */

		jmp			[pJmpTable+4*ebx]							/* go to code to handle situation */

		/* Compare flag Truth table
		 * results: if "a < b" is true, value is 1; 0 othersize.  
		 * Thus, if Y < X is true, that column gets a 1 in it.
		 * Nodes    Comparisons             	Truth table 
		 * AHEG:	Y <  X; Z <  X; Z <  Y		1  1  1
		 * AHEF:	Y <  X; Z <  X; Z >= Y		1  1  0
		 *       Y <  X; Z >= X; Z <  Y		1  0  1   - not possible
		 * AHBF:	Y <  X; Z >= X; Z >= Y		1  0  0
		 * AHCD:	Y >= X; Z >= X; Z <  Y		0  0  1
		 * AHBD:	Y >= X; Z >= X; Z >= Y		0  0  0
		 *       Y >= X; Z <  X; Z >= Y      0  1  0   - not possible
		 * AHCG:	Y >= X; Z <  X; Z <  Y		0  1  1
		 * incoming: mm0 = P000; mm1 = P111
		 * outgoing: X rslt = mm4; Y rslt = mm5; Z rslt = mm6
		 */

sse_Invalid:	/* this should never happen - compare error */
		jmp			sse_3dlut_100

sse_AHEG:
		/* AHEG:	Y < X;  Z < X;  Z < Y */
		/* (Pe  -Pa  )*X + (Pg  -Pe  )*Y + (Ph  -Pg  )*Z */
		/* (P100-P000)*X + (P110-P100)*Y + (P111-P110)*Z */
		movaps		xmm4, P110_SSE								/* xmm4 = P110 */
		movaps		xmm5, xmm7									/* xmm5 = P111 */

		movaps		xmm3, P100_SSE								/* xmm3 = P100 */

		subps		xmm5, xmm4									/* xmm5 = P111-P110 */

		subps		xmm4, xmm3									/* xmm4 = P110-P100 */
		mulps		xmm2, xmm5									/* xmm2 = (P111-P110)*Z */

		subps		xmm3, xmm6									/* xmm3 = P100-P000 */
		mulps		xmm1, xmm4									/* xmm2 = (P110-P100)*Y */

		mulps		xmm0, xmm3									/* xmm0 = (P100-P000)*X */
		jmp			sse_3dlut_100

sse_AHEF:
		/* AHEF:	Y < X;  Z < X;  Z >= Y */
		/* (Pe  -Pa  )*X + (Ph  -Pf  )*Y + (Pf  -Pe  )*Z */
		/* (P100-P000)*X + (P111-P101)*Y + (P101-P100)*Z */
		movaps		xmm5, P101_SSE								/* xmm5 = P101 */
		movaps		xmm4, xmm7									/* xmm4 = P111 */
		
		movaps		xmm3, P100_SSE								/* xmm3 = P100 */

		subps		xmm4, xmm5									/* xmm4 = P111-P101 */

		subps		xmm5, xmm3									/* xmm5 = P101-P100 */
		mulps		xmm1, xmm4									/* xmm1 = (P111-P101)*Y */

		subps		xmm3, xmm6									/* xmm3 = P100-P000 */
		mulps		xmm2, xmm5									/* xmm2 = (P101-P100)*Z */

		mulps		xmm0, xmm3									/* xmm0 = (P100-P000)*X */
		jmp			sse_3dlut_100

sse_AHBF:
		/* AHBF:	Y < X;  Z >= X;  Z >= Y */
		/* (Pf  -Pb  )*X + (Ph  -Pf  )*Y + (Pb  -Pa  )*Z */
		/* (P101-P001)*X + (P111-P101)*Y + (P001-P000)*Z */
		movaps		xmm3, P101_SSE								/* xmm3 = P101 */
		movaps		xmm4, xmm7									/* xmm4 = P111 */
		
		movaps		xmm5, P001_SSE								/* xmm5 = P001 */

		subps		xmm4, xmm3									/* xmm4 = P111-P101 */
		
		subps		xmm3, xmm5									/* xmm3 = P101-P001 */
		mulps		xmm1, xmm4									/* xmm1 = (P111-P101)*Y */

		subps		xmm5, xmm6									/* xmm5 = P001-P000 */
		mulps		xmm0, xmm3									/* xmm0 = (P101-P001)*X */

		mulps		xmm2, xmm5									/* xmm2 = (P001-P000)*Z */
		jmp			sse_3dlut_100
		
sse_AHCD:
		/* AHCD:	Y >= X;  Z >= X;  Z < Y */
		/* (Ph  -Pd  )*X + (Pc  -Pa  )*Y + (Pd  -Pc  )*Z */
		/* (P111-P011)*X + (P010-P000)*Y + (P011-P010)*Z */
		movaps		xmm5, P011_SSE								/* xmm5 = P011 */
		movaps		xmm3, xmm7									/* xmm3 = P111 */

		movaps		xmm4, P010_SSE								/* xmm4 = P010 */

		subps		xmm3, xmm5									/* xmm3 = P111-P011 */

		subps		xmm5, xmm4									/* xmm5 = P011-P010 */
		mulps		xmm0, xmm3									/* xmm0 = (P111-P011)*X */

		subps		xmm4, xmm6									/* xmm4 = P010-P000 */
		mulps		xmm2, xmm5									/* xmm2 = (P011-P010)*Z */
		
		mulps		xmm1, xmm4									/* xmm1 = (P010-P000)*Y */
		jmp			sse_3dlut_100

sse_AHBD:
		/* AHBD:	Y >= X;  Z >= X;  Z >= Y */
		/* (Ph  -Pd  )*X + (Pd  -Pb  )*Y + (Pb  -Pa  )*Z */
		/* (P111-P011)*X + (P011-P001)*Y + (P001-P000)*Z */
		movaps		xmm4, P011_SSE								/* xmm4 = P011 */
		movaps		xmm3, xmm7									/* xmm3 = P111 */

		movaps		xmm5, P001_SSE								/* xmm5 = P001 */

		subps		xmm3, xmm4									/* xmm3 = P111-P011 */
		
		subps		xmm4, xmm5									/* xmm4 = P011-P001 */
		mulps		xmm0, xmm3									/* xmm0 = (P111-P011)*X */

		subps		xmm5, xmm6									/* xmm5 = P001-P000 */
		mulps		xmm1, xmm4									/* xmm1 = (P011-P001)*Y */
		
		mulps		xmm2, xmm5									/* xmm2 = (P001-P000)*Z */
		jmp			sse_3dlut_100
		
sse_AHCG:
		/* AHCG:	Y >= X;  Z < X;  Z < Y */
		/* (Pg  -Pc  )*X + (Pc  -Pa  )*Y + (Ph  -Pg  )*Z */
		/* (P110-P010)*X + (P010-P000)*Y + (P111-P110)*Z */
		movaps		xmm3, P110_SSE								/* xmm3 = P110 */
		movaps		xmm5, xmm7									/* xmm5 = P111 */

		movaps		xmm4, P010_SSE								/* xmm4 = P010 */

		subps		xmm5, xmm3									/* xmm5 = P111-P110 */
		
		subps		xmm3, xmm4									/* xmm3 = P110-P010 */
		mulps		xmm2, xmm5									/* xmm2 = (P111-P110)*Z */

		subps		xmm4, xmm6									/* xmm4 = P010-P000 */
		mulps		xmm0, xmm3									/* xmm0 = (P110-P010)*X */
		
		mulps		xmm1, xmm4									/* xmm1 = (P010-P000)*Y */

		/* final calculations */
		/* for RGB: iRedMath(etc.) = (XStuff * XFrac) + (YStuff * YFrac) + (ZStuff * ZFrac) */
		/* pDstPixel->Red   = m_pOutputLut[P000->Red   + iRedMath                   ]; */
		/* pDstPixel->Green = m_pOutputLut[P000->Green + iGreenMath +   OUT_LUT_SIZE_SSE]; */
		/* pDstPixel->Blue  = m_pOutputLut[P000->Blue  + iBlueMath  + 2*OUT_LUT_SIZE_SSE]; */
sse_3dlut_100:
		/* xmm3 = values for X mult */
		/* xmm4 = values for Y mult */
		/* xmm5 = values for Z mult */
		mov			edi, pDstPixel								/* edi = pDstPixel */
		mov			edx, dstOffset								/* edx = dest offset */

		addps		xmm0, xmm1									/* xmm0 = RGB X results + RGB Y results */
		
		addps		xmm0, xmm2									/* xmm0 = RGB X + Y + Z results */
		mov			ecx, m_pOutputLut							/* ecx = m_pOutputLut; */

		addps		xmm0, xmm6									/* xmm0 = RGB X + Y + Z results + P000 */

		cvtps2pi	mm0,  xmm0									/* mm0  = RG part of result */
		movhlps		xmm1, xmm0									/* xmm1 = B part of result */

		cvtps2pi	mm1,  xmm1									/* mm1  = B part of result */
																     
		pextrw		eax, mm0, 0									/* eax  = R part */

		pextrw		ebx, mm0, 2									/* ebx  = G part */
		mov			ax, [ecx+2*eax]								/* ax  = R result */

		mov			[edi], ax									/* save R result */
		mov			bx, [ecx+2*ebx + (EVAL_OLUT_ENTRIES_FMT3*2)]	/* bx  = G result */
		pextrw		eax, mm1, 0									/* eax  = B part */

		mov			[edi+edx], bx								/* save G result */
		mov			ax, [ecx+2*eax + (2*EVAL_OLUT_ENTRIES_FMT3*2)]	/* ax  = B result */

		mov			[edi+2*edx], ax								/* save B result */

		add			edi, dstPelStride							/* next dest pixel */
		mov			pDstPixel, edi
		dec			i1											/* i1-- */

		jnz			sse_3dlut_40

		emms
		pop			ebx											/* restore frame ptr */
	}
}


/* tetrahedral interpolation using SSE2 */

/* address definitions for pixel cube corner access; use these for SSE math, too */
#define P000_SSE2 [edi]					/* base pointer */
#define P001_SSE2 [edi+SIZEOF_M128]
#define P010_SSE2 [edi+eax]				/* P010 offset in eax */
#define P011_SSE2 [edi+eax+SIZEOF_M128]	/* P010 + SIZEOF_M128 */
#define P100_SSE2 [edi+ecx]				/* P100 offset in ecx */
#define P101_SSE2 [edi+ecx+SIZEOF_M128]
#define P110_SSE2 [edi+esi]				/* P110 offset in esi */
#define P111_SSE2 [edi+esi+SIZEOF_M128]

/* values for SSE2 math calculations */

void evalTh1i3o3D12ISSE2 (	imagePtr_p	inp,
						KpInt32_p	inStride,
						KpUInt32_t	dataTypeI,
						imagePtr_p	outp,
						KpInt32_p	outStride,
						KpUInt32_t	dataTypeO,
						KpInt32_t	n,
						PTTable_p	PTTableP)
{
KpInt32_t	i1, srcOffset, srcPelStride, dstOffset, dstPelStride;
KpUInt8_p	pSrcPixel, pDstPixel;
evalILutSSE2_p	m_pInputLut;
KpUInt16_p	m_pOutputLut;
inx128_t	*pTemp;
KpUInt8_t	i128Space[SIZEOF_M128 * 3];	/* space for putting info. */
static		pFunction pJmpTable[8] = {0,0,0,0,0,0,0,0};
KpInt32_t   m_iP001, m_iP010, m_iP011, m_iP100, m_iP101, m_iP110, m_iP111;

	m_pInputLut = (evalILutSSE2_p)PTTableP->etLuts[ET_I12F2].P;
	m_pOutputLut = PTTableP->etLuts[ET_O12].P;

	m_iP001 = PTTableP->etGOffsets[1] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one up				(x+0, y+0, z+1) */
	m_iP010 = PTTableP->etGOffsets[2] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one back				(x+0, y+1, z+0) */
	m_iP011 = PTTableP->etGOffsets[3] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one up and back 		(x+0, y+1, z+1) */
	m_iP100 = PTTableP->etGOffsets[4] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right			(x+1, y+0, z+0) */
	m_iP101 = PTTableP->etGOffsets[5] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right and up		(x+1, y+0, z+1) */
	m_iP110 = PTTableP->etGOffsets[6] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right and back	(x+1, y+1, z+0) */
	m_iP111 = PTTableP->etGOffsets[7] / (3 * sizeof (mf2_tbldat_t)) * SIZEOF_M128;	/* one right, back, up	(x+1, y+1, z+1) */

	/* set up counter & pointer info */
	i1 = n;
	pSrcPixel = inp[0].p8;
	srcOffset = inp[1].p8 - pSrcPixel;
	srcPelStride = inStride[0];
	pDstPixel = outp[0].p8;
	dstOffset = outp[1].p8 - pDstPixel;
	dstPelStride = outStride[0];
	pDstPixel -= dstPelStride;	/* algorithm advances pointer before storage */

	/* set up MMX data */
	pTemp  = (inx128_t*)(ALIGN_PTR(i128Space, SIZEOF_M128));
	pTemp->m128i_u32[0] = (KpUInt32_t)EVAL_OLUT_INTERP_ROUND_FMT2;
	pTemp->m128i_u32[1] = (KpUInt32_t)EVAL_OLUT_INTERP_ROUND_FMT2;
	pTemp->m128i_u32[2] = (KpUInt32_t)EVAL_OLUT_INTERP_ROUND_FMT2;
	pTemp->m128i_u32[3] = (KpUInt32_t)0;
	(pTemp+1)->m128i_u16[0] = (KpUInt32_t)(1 << EVAL_OLUT_INTERPBITS_FMT2);
	(pTemp+1)->m128i_u16[1] = (KpUInt32_t)0;
	(pTemp+1)->m128i_u16[2] = (KpUInt32_t)(1 << EVAL_OLUT_INTERPBITS_FMT2);
	(pTemp+1)->m128i_u16[3] = (KpUInt32_t)0;
	(pTemp+1)->m128i_u16[4] = (KpUInt32_t)(1 << EVAL_OLUT_INTERPBITS_FMT2);
	(pTemp+1)->m128i_u16[5] = (KpUInt32_t)0;
	(pTemp+1)->m128i_u16[6] = (KpUInt32_t)(1 << EVAL_OLUT_INTERPBITS_FMT2);
	(pTemp+1)->m128i_u16[7] = (KpUInt32_t)0;

	_asm
	{
		push		ebx		/* save this guy - compiler bug for now */

		/* load up jump table
		 * AHEG:	Y <  X; Z <  X; Z <  Y		1  1  1
		 * AHEF:	Y <  X; Z <  X; Z >= Y		1  1  0
		 *			Y <  X; Z >= X; Z <  Y		1  0  1   - not possible
		 * AHBF:	Y <  X; Z >= X; Z >= Y		1  0  0
		 * AHCD:	Y >= X; Z >= X; Z <  Y		0  0  1
		 * AHBD:	Y >= X; Z >= X; Z >= Y		0  0  0
		 *			Y >= X; Z <  X; Z >= Y      0  1  0   - not possible
		 * AHCG:	Y >= X; Z <  X; Z <  Y		0  1  1
		 */
		cmp			dword ptr [pJmpTable], 0				/* skip if already initialized */
		jnz			sse2_3dlut_10

		mov			eax, offset sse2_AHEG

		mov			[pJmpTable + 7*4], eax					/* AHEG: 1  1  1 */
		mov			ebx, offset sse2_AHEF

		mov			[pJmpTable + 6*4], ebx					/* AHEF: 1  1  0 */
		mov			ecx, offset sse2_AHBF

		mov			[pJmpTable + 4*4], ecx					/* AHBF: 1  0  0 */
		mov			eax, offset sse2_AHCD

		mov			[pJmpTable + 1*4], eax					/* AHCD: 0  0  1 */
		mov			ebx, offset sse2_AHBD

		mov			[pJmpTable + 0*4], ebx					/* AHBD: 0  0  0 */
		mov			ecx, offset sse2_AHCG

		mov			[pJmpTable + 3*4], ecx					/* AHCG: 0  1  1 */
		mov			eax, offset sse2_Invalid

		mov			[pJmpTable + 5*4], eax					/* Bad:  1  0  1 */

		mov			[pJmpTable + 2*4], eax					/* Bad:  0  1  0 */

sse2_3dlut_10:

sse2_3dlut_40:												/*  do { */
		/* use input shaping LUT; base address of grid table is part of red (1st chan.) index.
		 * P000 = (Pixel*)(
		 *		m_pInputLut [pSrcPixel->Red                    ].index +  /* x
		 *		m_pInputLut [pSrcPixel->Green + IN_LUT_SIZE_D12    ].index +  /* y
		 *		m_pInputLut [pSrcPixel->Blue  + (2*IN_LUT_SIZE_D12)].index);  /* z
		 * Pa = P000; Pb = P001; Pc = P010; Pd = P011
		 * Pe = P100; Pf = P101; Pg = P110; Ph = P111

		 * get fractional parts of values for interpolation
		 * iXFrac = m_pInputLut [pSrcPixel->Red                    ].fraction;
		 * iYFrac = m_pInputLut [pSrcPixel->Green + IN_LUT_SIZE_D12    ].fraction;
		 * iZFrac = m_pInputLut [pSrcPixel->Blue  + (2*IN_LUT_SIZE_D12)].fraction;
		 *
		 * 2 issues when loading up fractions:
		 * - one set of XMM registers needs to be: 4 - x fraction, 4 - y fraction, 4 - z fraction
		 *   used for multiplying pixels by fractions in calculation
		 * - one set of XMM registers used for comparison: YZZ values and XXY values
		 *   compare YYZZ to YXXY values to get 3 nonzero flags represent compare results
		 *   move flags to register for use in indexed jump 
		 *   (Y compared to Y gives 0 for less than compare, so the largest bit is always 0)
		 */

		mov			esi, pSrcPixel								/* esi = pSrcPixel */
		mov			ecx, srcOffset								/* ecx = source offset */
		mov			edx, m_pInputLut							/* edx = table base */

		movzx		eax, word ptr [esi]							/* get red pixel value */
		movzx		ebx, word ptr [esi+ecx]						/* get green pixel value */
		and			eax, 0fffh									/* restrict data to 12 bits */
		pxor		xmm5, xmm5									/* xmm5 = 0 */

		mov			edi, [edx + 8*eax]							/* get starting index */
		and			ebx, 0fffh
		mov			eax, [edx + 8*eax + 4]						/* eax = iXFrac */

		add			edi, [edx + 8*ebx + ((1 << 12)*IN_LUT_ENTRY_SIZE_SSE2)]
																/* add in green index */
		movzx		ecx, word ptr [esi+2*ecx]					/* get blue pixel value */

		mov			ebx, [edx + 8*ebx + ((1 << 12)*IN_LUT_ENTRY_SIZE_SSE2) + 4]
		and			ecx, 0fffh
																/* ebx = iYFrac */

		add			edi, [edx + 8*ecx + (2*(1 << 12)*IN_LUT_ENTRY_SIZE_SSE2)]
		mov			ecx, [edx + 8*ecx + (2*(1 << 12)*IN_LUT_ENTRY_SIZE_SSE2) + 4]
																/* ecx = iZFrac */
		movd		xmm0, eax									/* xmm0 = X */
		add			esi, srcPelStride							/* next source pixel */

		mov			pSrcPixel, esi

		pshuflw		xmm0, xmm0,0								/* xmm0 = XXXX */
		movd		xmm1, ebx									/* xmm1 = Y */
		movd		xmm2, ecx									/* xmm2 = Z */

		/* SSE access definitions; shown here for convenient documentation; defined previously.
		 * #define P000_SSE [edi]						/* base pointer
		 * #define P001_SSE [edi+SIZEOF_M128]
		 * #define P010_SSE [edi+eax]					/* P010 offset in eax
		 * #define P011_SSE [edi+eax+SIZEOF_M128]		/* P010 + SIZEOF_M128
		 * #define P100_SSE [edi+ecx]					/* P100 offset in ecx
		 * #define P101_SSE [edi+ecx+SIZEOF_M128]
		 * #define P110_SSE [edi+esi]					/* P110 offset in esi
		 * #define P111_SSE [edi+esi+SIZEOF_M128]
		 */

		/* prepare fractions for multiplying and comparing */
		punpcklwd	xmm0, xmm5									/* xmm0 = 0X0X0X0X */
		xor			edx, edx									/* edx = empty jump table result */
		cmp			ebx, eax									/* Y < X ? */

		pshuflw		xmm1, xmm1,0								/* xmm1 = YYYY */
		rcl			edx, 1
		cmp			ecx, eax									/* Z < X ? */

		pshuflw		xmm2, xmm2,0								/* xmm2 = ZZZZ */
		rcl			edx, 1
		cmp			ecx, ebx									/* Z < Y ? */

		punpcklwd	xmm1, xmm5									/* xmm1 = 0Y0Y0Y0Y */
		rcl			edx, 1
		mov			eax, m_iP010								/* eax  = pixel P010 offset */

		movdqu		xmm6, P000_SSE2								/* xmm6  = P000->RGB */
		mov			esi, m_iP110								/* esi  = pixel P110 offset */
		mov			ebx, edx									/* ebx  = table index */

		movdqu		xmm7, P111_SSE2								/* xmm7  = P111->RGB */
		mov			ecx, m_iP100								/* ecx  = pixel P100 offset */
		mov			edx, m_pInputLut							/* edx = table base */

		punpcklwd	xmm2, xmm5									/* xmm2 = 0Z0Z0Z0Z */
		jmp			[pJmpTable+4*ebx]							/* go to code to handle situation */

		/* Compare flag Truth table
		 * results: if "a < b" is true, value is 1; 0 othersize.  
		 * Thus, if Y < X is true, that column gets a 1 in it.
		 * Nodes    Comparisons             	Truth table 
		 * AHEG:	Y <  X; Z <  X; Z <  Y		1  1  1
		 * AHEF:	Y <  X; Z <  X; Z >= Y		1  1  0
		 *       Y <  X; Z >= X; Z <  Y		1  0  1   - not possible
		 * AHBF:	Y <  X; Z >= X; Z >= Y		1  0  0
		 * AHCD:	Y >= X; Z >= X; Z <  Y		0  0  1
		 * AHBD:	Y >= X; Z >= X; Z >= Y		0  0  0
		 *       Y >= X; Z <  X; Z >= Y      0  1  0   - not possible
		 * AHCG:	Y >= X; Z <  X; Z <  Y		0  1  1
		 * incoming: mm0 = P000; mm1 = P111
		 * outgoing: X rslt = mm4; Y rslt = mm5; Z rslt = mm6
		 */

sse2_Invalid:	/* this should never happen - compare error */
		jmp			sse2_3dlut_100

sse2_AHEG:
		/* AHEG:	Y < X;  Z < X;  Z < Y */
		/* (Pe  -Pa  )*X + (Pg  -Pe  )*Y + (Ph  -Pg  )*Z */
		/* (P100-P000)*X + (P110-P100)*Y + (P111-P110)*Z */
		movdqa		xmm4, P110_SSE2								/* xmm4 = P110 */
		movdqa		xmm5, xmm7									/* xmm5 = P111 */

		movdqa		xmm3, P100_SSE2								/* xmm3 = P100 */

		psubd		xmm5, xmm4									/* xmm5 = P111-P110 */

		psubd		xmm4, xmm3									/* xmm4 = P110-P100 */
		pmaddwd		xmm2, xmm5									/* xmm2 = (P111-P110)*Z */

		psubd		xmm3, xmm6									/* xmm3 = P100-P000 */
		pmaddwd		xmm1, xmm4									/* xmm2 = (P110-P100)*Y */

		pmaddwd		xmm0, xmm3									/* xmm0 = (P100-P000)*X */
		jmp			sse2_3dlut_100


sse2_AHEF:
		/* AHEF:	Y < X;  Z < X;  Z >= Y */
		/* (Pe  -Pa  )*X + (Ph  -Pf  )*Y + (Pf  -Pe  )*Z */
		/* (P100-P000)*X + (P111-P101)*Y + (P101-P100)*Z */
		movdqa		xmm5, P101_SSE2								/* xmm5 = P101 */
		movdqa		xmm4, xmm7									/* xmm4 = P111 */
		
		movdqa		xmm3, P100_SSE2								/* xmm3 = P100 */

		psubd		xmm4, xmm5									/* xmm4 = P111-P101 */

		psubd		xmm5, xmm3									/* xmm5 = P101-P100 */
		pmaddwd		xmm1, xmm4									/* xmm1 = (P111-P101)*Y */

		psubd		xmm3, xmm6									/* xmm3 = P100-P000 */
		pmaddwd		xmm2, xmm5									/* xmm2 = (P101-P100)*Z */

		pmaddwd		xmm0, xmm3									/* xmm0 = (P100-P000)*X */
		jmp			sse2_3dlut_100

sse2_AHBF:
		/* AHBF:	Y < X;  Z >= X;  Z >= Y */
		/* (Pf  -Pb  )*X + (Ph  -Pf  )*Y + (Pb  -Pa  )*Z */
		/* (P101-P001)*X + (P111-P101)*Y + (P001-P000)*Z */
		movdqa		xmm3, P101_SSE2								/* xmm3 = P101 */
		movdqa		xmm4, xmm7									/* xmm4 = P111 */
		
		movdqa		xmm5, P001_SSE2								/* xmm5 = P001 */

		psubd		xmm4, xmm3									/* xmm4 = P111-P101 */
		
		psubd		xmm3, xmm5									/* xmm3 = P101-P001 */
		pmaddwd		xmm1, xmm4									/* xmm1 = (P111-P101)*Y */

		psubd		xmm5, xmm6									/* xmm5 = P001-P000 */
		pmaddwd		xmm0, xmm3									/* xmm0 = (P101-P001)*X */

		pmaddwd		xmm2, xmm5									/* xmm2 = (P001-P000)*Z */
		jmp			sse2_3dlut_100
		
sse2_AHCD:
		/* AHCD:	Y >= X;  Z >= X;  Z < Y */
		/* (Ph  -Pd  )*X + (Pc  -Pa  )*Y + (Pd  -Pc  )*Z */
		/* (P111-P011)*X + (P010-P000)*Y + (P011-P010)*Z */
		movdqa		xmm5, P011_SSE2								/* xmm5 = P011 */
		movdqa		xmm3, xmm7									/* xmm3 = P111 */

		movdqa		xmm4, P010_SSE2								/* xmm4 = P010 */

		psubd		xmm3, xmm5									/* xmm3 = P111-P011 */

		psubd		xmm5, xmm4									/* xmm5 = P011-P010 */
		pmaddwd		xmm0, xmm3									/* xmm0 = (P111-P011)*X */

		psubd		xmm4, xmm6									/* xmm4 = P010-P000 */
		pmaddwd		xmm2, xmm5									/* xmm2 = (P011-P010)*Z */
		
		pmaddwd		xmm1, xmm4									/* xmm1 = (P010-P000)*Y */
		jmp			sse2_3dlut_100

sse2_AHBD:
		/* AHBD:	Y >= X;  Z >= X;  Z >= Y */
		/* (Ph  -Pd  )*X + (Pd  -Pb  )*Y + (Pb  -Pa  )*Z */
		/* (P111-P011)*X + (P011-P001)*Y + (P001-P000)*Z */
		movdqa		xmm4, P011_SSE2								/* xmm4 = P011 */
		movdqa		xmm3, xmm7									/* xmm3 = P111 */

		movdqa		xmm5, P001_SSE2								/* xmm5 = P001 */

		psubd		xmm3, xmm4									/* xmm3 = P111-P011 */
		
		psubd		xmm4, xmm5									/* xmm4 = P011-P001 */
		pmaddwd		xmm0, xmm3									/* xmm0 = (P111-P011)*X */

		psubd		xmm5, xmm6									/* xmm5 = P001-P000 */
		pmaddwd		xmm1, xmm4									/* xmm1 = (P011-P001)*Y */
		
		pmaddwd		xmm2, xmm5									/* xmm2 = (P001-P000)*Z */
		jmp			sse2_3dlut_100
		
sse2_AHCG:
		/* AHCG:	Y >= X;  Z < X;  Z < Y */
		/* (Pg  -Pc  )*X + (Pc  -Pa  )*Y + (Ph  -Pg  )*Z */
		/* (P110-P010)*X + (P010-P000)*Y + (P111-P110)*Z */
		movdqa		xmm3, P110_SSE2								/* xmm3 = P110 */
		movdqa		xmm5, xmm7									/* xmm5 = P111 */

		movdqa		xmm4, P010_SSE2								/* xmm4 = P010 */

		psubd		xmm5, xmm3									/* xmm5 = P111-P110 */
		
		psubd		xmm3, xmm4									/* xmm3 = P110-P010 */
		pmaddwd		xmm2, xmm5									/* xmm2 = (P111-P110)*Z */

		psubd		xmm4, xmm6									/* xmm4 = P010-P000 */
		pmaddwd		xmm0, xmm3									/* xmm0 = (P110-P010)*X */
		
		pmaddwd		xmm1, xmm4									/* xmm1 = (P010-P000)*Y */

		/* final calculations */
		/* for RGB: iRedMath(etc.) = (XStuff * XFrac) + (YStuff * YFrac) + (ZStuff * ZFrac) */
		/* pDstPixel->Red   = m_pOutputLut[P000->Red   + iRedMath                   ]; */
		/* pDstPixel->Green = m_pOutputLut[P000->Green + iGreenMath +   OUT_LUT_SIZE_SSE]; */
		/* pDstPixel->Blue  = m_pOutputLut[P000->Blue  + iBlueMath  + 2*OUT_LUT_SIZE_SSE]; */
sse2_3dlut_100:
		/* xmm3 = values for X mult */
		/* xmm4 = values for Y mult */
		/* xmm5 = values for Z mult */

		pslld		xmm6, EVAL_FMT2_FRACBITS					/* xmm6 = P000, up shifted by fraction bits */
		mov			edi, pDstPixel								/* edi = pDstPixel */
		mov			edx, dstOffset								/* edx = dest offset */

		mov			ecx, m_pOutputLut							/* ecx = m_pOutputLut */
		paddd		xmm0, xmm1									/* xmm0 = RGB X results + RGB Y results */

		paddd		xmm0, xmm2									/* xmm0 = RGB X + Y + Z results */
		mov			ebx, pTemp									/* point to temp data */

		paddd		xmm0, xmm6									/* xmm0 = RGB X + Y + Z results + P000; shifted up */
		movdqa		xmm5, [ebx]									/* xmm5 = rounding value for interpolation */
		movdqa		xmm4, [ebx+16];								/* xmm4 = scaled "1" values to subtract from (to give 1-fraction) */

		movdqa		xmm7, xmm0									/* xmm7 = fractional part */

		psrld		xmm0, EVAL_OLUT_FRACBITS_FMT2				/* xmm6 = integer part of RGB */
		pslld		xmm7, 32 - EVAL_OLUT_FRACBITS_FMT2			/* xmm7 = zero integer bits */

		pextrw		eax, xmm0, 4								/* eax  = integer B part */
		psrld		xmm7, 32 - EVAL_OLUT_INTERPBITS_FMT2

		movd		xmm1, [ecx+2*eax + (2*EVAL_OLUT_ENTRIES_FMT2*2)]	/* xmm1  = base&next from B ouput lut */
		psubw		xmm4, xmm7									/* xmm4 = 1-fraction */
		pslldq		xmm1, 4										/* make space for G base&next */

		pextrw		ebx, xmm0, 2								/* ebx  = integer G part */
		pslld		xmm7, 16									/* xmm7 = fraction shifted to hi 1/2 of dword */
		
		movd		xmm2, [ecx+2*ebx + (EVAL_OLUT_ENTRIES_FMT2*2)]	/* xmm2 = base&next from G ouput lut */
		por			xmm1, xmm2									/* xmm1 = B base&next and G base&next */
		por			xmm7, xmm4									/* xmm7 = (fraction), (1-fraction) in each dword section */
	
		pslldq		xmm1, 4										/* make space for R base&next */

		pextrw		eax, xmm0, 0								/* eax  = integer R part */

		add			edi, dstPelStride							/* next pDstPixel */
		movd		xmm2, [ecx+2*eax]							/* xmm2 = base&next from R ouput lut */

		por			xmm1, xmm2									/* xmm1 = B base&next, G base&next, and R base&next */

		mov			pDstPixel, edi
		pmaddwd		xmm1, xmm7									/* xmm1 = frac * (next - base) */

		paddd		xmm1, xmm5									/* round */
		psrld		xmm1, EVAL_OLUT_INTERPBITS_FMT2				/* get integer result */

		dec			i1											/* i1-- */
		pextrw		eax, xmm1, 0								/* eax  = integer R part */
		mov			[edi], ax									/* save R result */

		pextrw		ebx, xmm1, 2								/* ebx  = integer G part */
		mov			[edi+edx], bx								/* save G result */

		pextrw		eax, xmm1, 4								/* eax  = integer B part */
		mov			[edi+2*edx], ax								/* save B result */

		jnz			sse2_3dlut_40								/* } while (--i1); */

		emms
		pop			ebx											/* restore frame ptr */
	}
}

#endif
