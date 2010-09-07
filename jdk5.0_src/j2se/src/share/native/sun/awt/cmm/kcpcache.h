/*
 * @(#)kcpcache.h	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *	kcpcache.h

	Contains:	interface to color processors' special evaluations

	COPYRIGHT (c) 1995-2003 Eastman Kodak Company
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#ifndef _KCPCACHE_H_
#define _KCPCACHE_H_ 1

#include "kcptmgr.h"
#include "attrib.h"


#if defined (KPALPHA)
#define KCP_SHIFT_RIGHT(data, result, bits)  \
		if (data < 0) { \
			result = -((-data) >> (bits)); \
		} \
		else { \
			result = (data >> (bits)); \
		}

#define KCP_SHIFT_RIGHT_ROUND(data, result, bits)  \
		if ((data + (((1 << (bits)) -1) >> 1)) < 0) { \
			result = -(((-data + ((1 << (bits -1)) -1) -1)) >> (bits)); \
		} \
		else { \
			result = ((data + ((1 << (bits -1)) -1)) >> (bits)); \
		}
#else
#define KCP_SHIFT_RIGHT(data, result, bits) result = data >> (bits);

#define KCP_SHIFT_RIGHT_ROUND(data, result, bits) result = ((data + ((1 << (bits -1)) -1)) >> (bits));

#endif

#define KCP_EXTRACT_COMPONENT(data, dataBits, position) \
	((data >> position) & ((1 << dataBits) -1))

#define KCP_CONVERT_DOWN(data, startBits, endBits) \
	((data + ((1 << (startBits - endBits -1)) - (data >> endBits))) >> (startBits - endBits))

/* the following macro will only work if (startBits - (endBits - startBits)) >= 0
 * which means endBits can be no more than twice startBits
 * Use the macro successively if this condition is not met */
#define KCP_CONVERT_UP(data, startBits, endBits) \
((data << (endBits-startBits)) + ((data >> (startBits - (endBits - startBits)))))

/* The overhead associated with optimizing the tables means that below some
	threshold evaluation will be faster using non-optimized evaluation */
#define TH1_MIN_EVALS (1500)
#define TH1_MIN_16BIT_EVALS (15000)

#define EVAL_D16_FRACBITS	(20)		/* fractional bits for interpolations */
#define EVAL_D8_FRACBITS	(16)		/* fractional bits for interpolations */
#define EVAL_FMT2_FRACBITS	(15)		/* fractional bits for input table format 2 */

#define EVAL_OLUT_ENTRIESD8 (1 << 14)	/* size of 8 bit evaluation output lut */
#define EVAL_OLUT_ENTRIESD16 (1 << 16)	/* size of 12 & 16 bit evaluation output lut */

#define EVAL_OLUT_BITS_FMT2 12			/* bits of format 2 output lut */
#define EVAL_OLUT_FRACBITS_FMT2 15		/* fractional bits format 2 output lut indexing; EVAL_OLUT_BITS_FMT2 + EVAL_FMT2_FRACBITS - EVAL_OLUT_BITS_FMT2) assembler does not allow parenthesis */
#define EVAL_OLUT_INTERPBITS_FMT2 12	/* fractional bits format 2 output lut interpolation */
#define EVAL_OLUT_ENTRIES_FMT2 (1 << EVAL_OLUT_BITS_FMT2)	/* size of format 2 output lut */
#define EVAL_OLUT_INTERP_ROUND_FMT2		(1 << (EVAL_OLUT_INTERPBITS_FMT2-1))	/* rounding value for integer SSE2 math */

#define EVAL_OLUT_ENTRIES_FMT3 (1 << 16)	/* size of format 3 output lut */

#define ET_I8		(0)
#define ET_I12F1	(1)		/* index with shifted offsets; fraction with EVAL_D16_FRACBITS */
#define ET_I12F2	(2)		/* index with shifted offsets and grid table base; fraction with EVAL_SSE2_FRACBITS */
#define ET_I12F3	(3)		/* index with shifted offsets and grid table base; fraction is float */
#define ET_I16		(4)
#define ET_G12F1	(5)		/* entries are integers */
#define ET_GF2		(6)		/* entries are integers with dummy 4th channel */
#define ET_GF3		(7)		/* entries are floats with dummy 4th channel */
#define ET_G16		(8)
#define ET_O8		(9)
#define ET_O12		(10)
#define ET_O16		(11)

/***** definitions for processor types **************************
/*     Generally, each more capable processor type has a greater */
/*     value than the one before it. */
#define	NON_MMX					(0)		/* non mmx processor */
#define MMX_PRESENT				(1)		/* MMX processor present */
#define SSE_PRESENT				(2)		/* SSE (PIII) processor present */
#define SSE2_PRESENT			(3)		/* SSE2 (Willamette) processor present */
#define AMD_3DNOW_PRESENT		(4)		/* 3D Now processor present */
#define AMD_3DNOW_EXT_PRESENT	(5)		/* 3D Now extensions present */
#define MAC_ALTIVEC_PRESENT		(6)		/* MAC altivec instructions present */
#define SUN_VIS_PRESENT			(7)		/* Sun VIS instructions present */

#define	ET_ITBL_FMT1	(0)		/* integer format */
#define ET_ITBL_FMT2	(1)		/* index with shifted offsets and grid table base; fraction with EVAL_SSE2_FRACBITS */
#define ET_ITBL_FMT3	(2)		/* index with shifted offsets and grid table base; fraction is float */
#define ET_GTBL_FMT1	(3)		/* entries are integers */
#define ET_GTBL_FMT2	(4)		/* entries are integers with dummy 4th channel */
#define ET_GTBL_FMT3	(5)		/* entries are floats with dummy 4th channel */

/* The following macro aligns pointers to a byte boundary */
#define ALIGN_PTR(initial_addr, byte_boundary) \
	((((KpUInt32_t) initial_addr) + (byte_boundary-1)) & (KpUInt32_t)(~(KpUInt32_t)(byte_boundary-1)))

/* image pointers definition */
typedef union imagePtr_u {
	PTImgAddr_t pI;
	KpUInt8_p	p8;
	KpUInt16_p	p16;
	KpUInt32_p	p32;
} imagePtr_t, FAR* imagePtr_p;

typedef struct evalILuti_s {
	KpInt32_t	index;	/* index into grid table */
	KpInt32_t	frac;	/* the interpolant */
} evalILuti_t, FAR* evalILuti_p;

#if defined KCP_PENTIUM
/* Here we refer to the types __m128 and __m128i, defined in the Pentium processor pack: */
typedef __m128	flx128_t;
typedef __m128i	inx128_t;

typedef struct evalILutSSE_s {
	KpInt32_t	index;	/* index into grid table */
	float		frac;	/* the interpolant for SSE */
} evalILutSSE_t, FAR* evalILutSSE_p;
#define IN_LUT_ENTRY_SIZE_SSE	(8)	/* size of element for evalILutSSE_t; assembler can not handle sizeof(evalILutSSE_t) */

typedef struct evalILutSSE2_s {
	KpInt32_t	index;	/* index into grid table */
	KpInt32_t	frac;	/* the interpolant for SSE2 */
} evalILutSSE2_t, FAR* evalILutSSE2_p;
#define IN_LUT_ENTRY_SIZE_SSE2 (8)	/* size of element for evalILutSSE2_t; assembler can not handle sizeof(evalILutSSE2_t) */

typedef float evalGLutSSE_t, FAR* evalGLutSSE_p;		/* SSE grid table entry type */
typedef KpUInt32_t evalGLutSSE2_t, FAR* evalGLutSSE2_p;	/* SSE2 grid table entry type */
#endif

#define SIZEOF_M128	(16)	/* size of element for flx128_t and inx128_t; assembler can not handle sizeof() */

/* Evaluation output lut definition  */
typedef union evalOLutPtr_u {
	KpUInt8_p	p8;
	KpUInt16_p	p16;
	KpUInt32_p	p32;
} evalOLutPtr_t, FAR* evalOLutPtr_p; 

typedef void (*evalTh1Proc_t) (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
typedef evalTh1Proc_t FAR* evalTh1Proc_p;
typedef void (*formatFunc_t) (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

typedef struct evalControl_s {
	callBack_p		callBack;
	evalTh1Proc_t	evalFunc;
	KpInt32_t		nFuts;
	PTTable_p*		evalList;
	KpUInt32_t		ioMaskList[FUT_NOCHAN];
	KpUInt32_t		compatibleDataType;
	KpUInt32_t		optimizedEval;		/* 0 = no optimized tables, else = igo tables optimized */
#if defined KCP_PENTIUM
	KpUInt32_t		hasSSE;				/* Pentium processors */
	KpUInt32_t		hasSSE2;
#endif
	KpUInt32_t		iLutFormat, gLutFormat;			/* evaluation table formats */
	KpUInt32_t		iLutEntrySize, gLutEntrySize;	/* size of a lut entry */
	KpUInt32_t		evalDataTypeI, evalDataTypeO;
	KpInt32_t		imageLines, imagePels;
	imagePtr_t		inputData[FUT_NICHAN], outputData[FUT_NOCHAN];
	KpInt32_t		inPelStride[FUT_NICHAN], inLineStride[FUT_NICHAN];
	KpInt32_t		outPelStride[FUT_NOCHAN], outLineStride[FUT_NOCHAN];

	/* needed only if image formats do not match an optimized function */
	formatFunc_t	formatFuncI, formatFuncO;
	KpInt32_t		tempPelStride[FUT_NOCHAN];
	
	/* needed for returning MP errors */
	PTErr_t			PTErr;
} evalControl_t, FAR* evalControl_p, FAR* FAR* evalControl_h;


PTErr_t		allocEvalState (PTTable_p);
void		unlockEvalTables (PTTable_p);
PTErr_t		getEvalFunc (KpInt32_t, KpUInt32_t, KpUInt32_t, evalControl_p);
PTErr_t		initEvalTables (evalControl_p);
PTErr_t		evalImageMP (evalControl_p);
PTErr_t		evalImage (evalControl_p);

#ifdef  __cplusplus
extern "C" {
#endif
KpInt32_t	DetectProcessor (void);
#ifdef  __cplusplus
}
#endif

void evalTh1gen (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o1d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o1d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o2d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o2d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iB24oB24 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iL24oL24 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iQDoQD (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iQDo3 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3oQD (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3oXd16n (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o4d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o4d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o5d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o5d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o6d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o6d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o7d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o7d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o8d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o8d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3D12ISSE (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3D12ISSE2 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3D12PSSE (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3D12PSSE2 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);

/* Tetrahedral Interpolation evaluation of 3-channel 8-bit to 3-channel 12/16-bit data */
void evalTh1i3o3d8to16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);

/* Tetrahedral Interpolation evaluation of 3-channel 12/16-bit to 3-channel 8-bit data */
void evalTh1i3o3d16to8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i3o3d16to8QS (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p,  KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);

void evalTh1i4o1d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o1d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o2d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o2d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o3d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o3d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o3QD (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iB32oB32 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1iL32oL32 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o4d8 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4o4d16 (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);
void evalTh1i4oXd16n (imagePtr_p, KpInt32_p, KpUInt32_t, imagePtr_p, KpInt32_p, KpUInt32_t, KpInt32_t, PTTable_p);

void format555to8 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format565to8 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void format555to12 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format565to12 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format8to12 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format10to12 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void format555to16 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format565to16 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format8to16 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format10to16 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format12to16 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void pass8in (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void pass16in (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void format8to555 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format8to565 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void format12to555 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format12to565 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format12to8 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format12to10 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void format16to555 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format16to565 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format16to8 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format16to10 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void format16to12 (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

void pass8out (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);
void pass16out (KpInt32_t, imagePtr_p, KpInt32_p, imagePtr_p);

#endif	/* _KCPCACHE_H_ */
