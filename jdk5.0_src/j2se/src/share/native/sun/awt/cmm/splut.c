/*
 * @(#)splut.c	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*******************************************************************/
/* @(#)splut.c	1.28 99/02/16
	Contains:	This module contains functions for lut manipulation.

				Created by lsh, November 5, 1993

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1993-2002
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include <string.h>
#include <stdio.h>
#include "sprof-pr.h"
#include "attrcipg.h"

#define	CLUT_HEADER 20

/* need to fix 64-bit problem with pointer truncation */
#if defined(_M_IA64)
	#define INTPTR __int64
#else
	#define INTPTR int
#endif

/* Prototypes */
static void SpInitMatrix (
				KpF15d16_t FAR Matrix3x3 [9]);

static void SpInitMatrix12 (
				KpF15d16_t FAR *Matrix);

static KpUInt32_t SpLut8SizeOfInputTable (
				SpLut8Bit_t	FAR *Lut8);

static KpUInt32_t SpLut8SizeOfOutputTable (
				SpLut8Bit_t	FAR *Lut8);
				
static KpUInt32_t SpLut8SizeOfClut (
				SpLut8Bit_t	FAR *Lut8);
				
static SpStatus_t SpLut8ToPublic (
				char	KPHUGE *Buf,
				SpLut_t	FAR *Lut);
				
static SpStatus_t SpLut8FromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize);
				
static KpUInt32_t SpLut16SizeOfInputTable (
				SpLut16Bit_t	FAR *Lut16);

static KpUInt32_t SpLut16SizeOfOutputTable (
				SpLut16Bit_t	FAR *Lut16);

static KpUInt32_t SpLut16SizeOfClut (
				SpLut16Bit_t	FAR *Lut16);

static SpStatus_t SpLut16ToPublic (
				char		KPHUGE *Buf,
				SpLut_t		FAR *Lut);
				
static SpStatus_t SpLut16FromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize);

static SpStatus_t SpLutABToPublic (
				char		KPHUGE *Buf,
				SpLut_t		FAR *Lut);
				
static SpStatus_t SpLutBAToPublic (
				char		KPHUGE *Buf,
				SpLut_t		FAR *Lut);
				
static SpStatus_t SpLutABFromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize);
				
static SpStatus_t SpLutBAFromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize);
				


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Initialize matrix.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	December 25, 1993
 *------------------------------------------------------------------*/
static void SpInitMatrix (KpF15d16_t FAR Matrix3x3 [9])
{
	int	i;

	Matrix3x3 [0] = KpF15d16FromDouble (0.0);
	for (i = 1; i < 9; i++)
		Matrix3x3 [i] = Matrix3x3 [0];

	Matrix3x3 [0] =
	Matrix3x3 [4] =
	Matrix3x3 [8] = KpF15d16FromDouble (1.0);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Initialize New Lut matrix.
 *
 * AUTHOR
 * 	lsh - modified by doro
 *
 * DATE MODIFIED
 *	April 17, 2001
 *------------------------------------------------------------------*/
static void SpInitMatrix12 (KpF15d16_t FAR *Matrix)
{
	int	i;

	Matrix [0] = KpF15d16FromDouble (0.0);
	for (i = 1; i < 12; i++)
		Matrix [i] = Matrix [0];

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of InputTable for 8 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut8SizeOfInputTable (
				SpLut8Bit_t	FAR *Lut8)
{
	return 256 * Lut8->InputChannels;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of OutputTable for 8 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut8SizeOfOutputTable (
				SpLut8Bit_t	FAR *Lut8)
{
	return 256 * Lut8->OutputChannels;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of clut for 8 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut8SizeOfClut (
				SpLut8Bit_t	FAR *Lut8)
{
	KpUInt32_t	Size;
	KpUInt32_t	i;

	Size = Lut8->LUTDimensions;
	for (i = 1; i < Lut8->InputChannels; i++)
		Size *= Lut8->LUTDimensions;

	return Size * Lut8->OutputChannels;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create empty 8 Bit Lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpLut8Create (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt16_t	LUTDimensions,
				SpLut_t		FAR *Lut)
{
	KpUInt32_t	Size;
	SpStatus_t	Status;
	SpLut8Bit_t	FAR *Lut8;

/* validate parameters */
	if ((InChannels < 1) || (8 < InChannels))
		return SpStatOutOfRange;

	if ((OutChannels < 1) || (8 < OutChannels))
		return SpStatOutOfRange;

	if ((LUTDimensions < 2) || (SPMAXLUTSIZE < LUTDimensions))
		return SpStatOutOfRange;

/* initialize all structure members */
	Lut->LutType = SpTypeLut8;
	Lut8 = &Lut->L.Lut8;
	Lut8->InputChannels = (char) InChannels;
	Lut8->OutputChannels = (char) OutChannels;
	Lut8->LUTDimensions = (char) LUTDimensions;
	SpInitMatrix (Lut8->Matrix3x3);
	Lut8->InputTable = NULL;
	Lut8->CLUT = NULL;
	Lut8->OutputTable = NULL;

	Status = SpStatSuccess;

/* input table */
	Size = SpLut8SizeOfInputTable (Lut8);
	Lut8->InputTable = SpMalloc (Size);
	if (NULL == Lut8->InputTable)
		Status = SpStatMemory;

/* CLUT */
	Size = SpLut8SizeOfClut (Lut8);
	Lut8->CLUT = SpMalloc (Size);
	if (NULL == Lut8->CLUT)
		Status = SpStatMemory;

/* output table */
	Size = SpLut8SizeOfOutputTable (Lut8);
	Lut8->OutputTable = SpMalloc (Size);
	if (NULL == Lut8->OutputTable)
		Status = SpStatMemory;

/* check for error */
	if (SpStatSuccess != Status) {
		SpFree (Lut8->CLUT);
		Lut8->CLUT = NULL;
		SpFree (Lut8->InputTable);
		Lut8->InputTable = NULL;
		SpFree (Lut8->OutputTable);
		Lut8->OutputTable = NULL;
	}

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lut8 to public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static SpStatus_t SpLut8ToPublic (
				char	KPHUGE *Buf,
				SpLut_t	FAR *Lut)
{
	SpLut8Bit_t	FAR *Lut8;
	SpStatus_t	Status;

	Lut8 = &Lut->L.Lut8;

/* get fixed size elements */
	Lut8->InputChannels = *Buf++;
	Lut8->OutputChannels = *Buf++;
	Lut8->LUTDimensions = *Buf++;

/* varify pad byte is zero */
	if (0 != *Buf++)
		return SpStatOutOfRange;

/* allocate arrays */
	Status = SpLut8Create (Lut8->InputChannels, Lut8->OutputChannels,
								Lut8->LUTDimensions, Lut);
	if (SpStatSuccess != Status)
		return Status;

/* get variable length stuff */
	SpGetF15d16 (&Buf, Lut8->Matrix3x3, 9);
	SpGetBytes (&Buf, Lut8->InputTable, SpLut8SizeOfInputTable (Lut8));
	SpGetBytes (&Buf, Lut8->CLUT, SpLut8SizeOfClut (Lut8));
	SpGetBytes (&Buf, Lut8->OutputTable, SpLut8SizeOfOutputTable (Lut8));

	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert 8bit lut from public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static SpStatus_t SpLut8FromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize)
{
	SpLut8Bit_t	FAR *Lut8;
	char		KPHUGE *Buf;
	KpUInt32_t	ClutSize;
	KpUInt32_t	InputTableSize;
	KpUInt32_t	OutputTableSize;

	Lut8 = &Lut->L.Lut8;

/* space for fixed size components */
	*BufferSize = 12 + 9 * sizeof (KpF15d16_t);

/* input table */
	InputTableSize = SpLut8SizeOfInputTable (Lut8);
	*BufferSize += InputTableSize;

/* CLUT */
	ClutSize = SpLut8SizeOfClut (Lut8);
	*BufferSize += ClutSize;

/* output table */
	OutputTableSize = SpLut8SizeOfOutputTable (Lut8);
	*BufferSize += OutputTableSize;

/* allocate a buffer */
	Buf = SpMalloc (*BufferSize);
	if (NULL == Buf)
		return SpStatMemory;

/* fill the buffer */
	*Buffer = Buf;
	SpPutUInt32 (&Buf, SpTypeLut8);
	SpPutUInt32 (&Buf, 0L);
	*Buf++ = Lut8->InputChannels;
	*Buf++ = Lut8->OutputChannels;
	*Buf++ = Lut8->LUTDimensions;
	*Buf++ = '\0';
	SpPutF15d16 (&Buf, Lut8->Matrix3x3, 9);
	SpPutBytes (&Buf, InputTableSize, Lut8->InputTable);
	SpPutBytes (&Buf, ClutSize, Lut8->CLUT);
	SpPutBytes (&Buf, OutputTableSize, Lut8->OutputTable);
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of InputTable for 16 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut16SizeOfInputTable (
				SpLut16Bit_t	FAR *Lut16)
{
	return ((KpUInt32_t)sizeof (KpUInt16_t) * 
                        Lut16->InputTableEntries * 
                        Lut16->InputChannels);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of OutputTable for 16 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut16SizeOfOutputTable (
				SpLut16Bit_t	FAR *Lut16)
{
	return ((KpUInt32_t)sizeof (KpUInt16_t) * 
                      Lut16->OutputTableEntries * 
                      Lut16->OutputChannels);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Compute size of clut for 16 bit lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static KpUInt32_t SpLut16SizeOfClut (
				SpLut16Bit_t	FAR *Lut16)
{
	KpUInt32_t	Size;
	KpUInt32_t	i;

	Size = Lut16->LUTDimensions;
	for (i = 1; i < Lut16->InputChannels; i++)
		Size *= Lut16->LUTDimensions;

	return (Size * Lut16->OutputChannels * 
                       (KpUInt32_t)sizeof (KpUInt16_t));
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create empty 16 Bit Lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpLut16Create (
				KpUInt16_t	InChannels,
				KpUInt16_t	InputTableEntries,
				KpUInt16_t	OutChannels,
				KpUInt16_t	OutputTableEntries,
				KpUInt16_t	LUTDimensions,
				SpLut_t		FAR *Lut)
{
	SpLut16Bit_t	FAR *Lut16;
	KpUInt32_t		Size;
	SpStatus_t		Status;

/* validate parameters */
	if ((InChannels < 1) || (8 < InChannels))
		return SpStatOutOfRange;

	if ((OutChannels < 1) || (8 < OutChannels))
		return SpStatOutOfRange;

	if ((LUTDimensions < 2) || (SPMAXLUTSIZE < LUTDimensions))
		return SpStatOutOfRange;

	if ((InputTableEntries < 1) || (SPMAXTABLESIZE < InputTableEntries))
		return SpStatOutOfRange;

	if ((OutputTableEntries < 1) || (SPMAXTABLESIZE < OutputTableEntries))
		return SpStatOutOfRange;

/* initialize all structure members */
	Lut->LutType = SpTypeLut16;
	Lut16 = &Lut->L.Lut16;
	Lut16->InputChannels = (char) InChannels;
	Lut16->OutputChannels = (char) OutChannels;
	Lut16->LUTDimensions = (char) LUTDimensions;
	SpInitMatrix (Lut16->Matrix3x3);
	Lut16->InputTableEntries = InputTableEntries;
	Lut16->InputTable = NULL;
	Lut16->CLUT = NULL;
	Lut16->OutputTableEntries = OutputTableEntries;
	Lut16->OutputTable = NULL;

	Status = SpStatSuccess;

/* input table */
	Size = SpLut16SizeOfInputTable (Lut16);
	Lut16->InputTable = SpMalloc (Size);
	if (NULL == Lut16->InputTable)
		Status = SpStatMemory;

/* CLUT */
	Size = SpLut16SizeOfClut (Lut16);
	Lut16->CLUT = SpMalloc (Size);
	if (NULL == Lut16->CLUT)
		Status = SpStatMemory;

/* output table */
	Size = SpLut16SizeOfOutputTable (Lut16);
	Lut16->OutputTable = SpMalloc (Size);
	if (NULL == Lut16->OutputTable)
		Status = SpStatMemory;

/* check for error */
	if (SpStatSuccess != Status) {
		SpFree (Lut16->CLUT);
		Lut16->CLUT = NULL;
		SpFree (Lut16->InputTable);
		Lut16->InputTable = NULL;
		SpFree (Lut16->OutputTable);
		Lut16->OutputTable = NULL;
	}

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert 16bit lut to public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static SpStatus_t SpLut16ToPublic (
				char		KPHUGE *Buf,
				SpLut_t		FAR *Lut)
{
	SpLut16Bit_t	FAR *Lut16;
	KpUInt32_t		InputTableSize;
	KpUInt32_t		OutputTableSize;
	KpUInt32_t		ClutSize;
	SpStatus_t		Status;
	KpF15d16_t		Matrix3x3 [9];
	int			i;

	Lut16 = &Lut->L.Lut16;

/* get fixed size elements */
	Lut16->InputChannels = *Buf++;
	Lut16->OutputChannels = *Buf++;
	Lut16->LUTDimensions = *Buf++;

/* varify pad byte is zero */
	if (0 != *Buf++)
		return SpStatOutOfRange;

/* get the matrix */
	SpGetF15d16 (&Buf, Matrix3x3, 9);

/* get number of table entries */
	Lut16->InputTableEntries = SpGetUInt16 (&Buf);
	Lut16->OutputTableEntries = SpGetUInt16 (&Buf);

/* allocate arrays */
	Status = SpLut16Create (Lut16->InputChannels, Lut16->InputTableEntries,
							Lut16->OutputChannels, Lut16->OutputTableEntries,
							Lut16->LUTDimensions, Lut);
	if (SpStatSuccess != Status)
		return Status;

/* No store Matrix since Create initializes it */
	for (i = 0; i < 9; i++)
		Lut16->Matrix3x3[i] = Matrix3x3[i];

/* get arrays */
	InputTableSize = SpLut16SizeOfInputTable (Lut16);
	SpGetUInt16s (&Buf, Lut16->InputTable, InputTableSize / 2);

	ClutSize = SpLut16SizeOfClut (Lut16);
	SpGetUInt16s (&Buf, Lut16->CLUT, ClutSize / 2);

	OutputTableSize = SpLut16SizeOfOutputTable (Lut16);
	SpGetUInt16s (&Buf, Lut16->OutputTable, OutputTableSize / 2);

	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert 16bit lut from public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
static SpStatus_t SpLut16FromPublic (
				SpLut_t		FAR *Lut,
				void		KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize)
{
	SpLut16Bit_t	FAR *Lut16;
	char			KPHUGE	*Buf;
	KpUInt32_t		ClutSize;
	KpUInt32_t		InputTableSize;
	KpUInt32_t		MatrixSize;
	KpUInt32_t		OutputTableSize;

	Lut16 = &Lut->L.Lut16;

/* space for fixed size components */
	*BufferSize = 12 + 2 + 2;

/* matrix */
	MatrixSize = 9 * sizeof (KpF15d16_t);
	*BufferSize += MatrixSize;

/* input table */
	InputTableSize = SpLut16SizeOfInputTable (Lut16);
	*BufferSize += InputTableSize;

/* CLUT */
	ClutSize = SpLut16SizeOfClut (Lut16);
	*BufferSize += ClutSize;

/* output table */
	OutputTableSize = SpLut16SizeOfOutputTable (Lut16);
	*BufferSize += OutputTableSize;

/* allocate a buffer */
	Buf = SpMalloc (*BufferSize);
	if (NULL == Buf)
		return SpStatMemory;

/* fill the buffer */
	*Buffer = Buf;
	SpPutUInt32 (&Buf, SpTypeLut16);
	SpPutUInt32 (&Buf, 0L);
	*Buf++ = Lut16->InputChannels;
	*Buf++ = Lut16->OutputChannels;
	*Buf++ = Lut16->LUTDimensions;
	*Buf++ = '\0';
	SpPutF15d16 (&Buf, Lut16->Matrix3x3, 9);
	SpPutUInt16 (&Buf, Lut16->InputTableEntries);
	SpPutUInt16 (&Buf, Lut16->OutputTableEntries);
	SpPutUInt16s (&Buf, Lut16->InputTable, InputTableSize / 2);
	SpPutUInt16s (&Buf, Lut16->CLUT, ClutSize / 2);
	SpPutUInt16s (&Buf, Lut16->OutputTable, OutputTableSize / 2);
	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get Curve
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpGetABCurve (char	FAR * KPHUGE *Buf,
				SpLutCurve_t	* FAR *Curve,
				KpUInt32_t	ChanCount)
{
KpUInt32_t	Index;
SpStatus_t	Status;
SpLutCurve_t	*lCurve;

/* allocate array pointers*/
	lCurve = *Curve;

	for (Index = 0; Index < ChanCount; Index++)
	{
		lCurve[Index].CurveType = SpGetUInt32(Buf);
		lCurve[Index].Reserved = SpGetUInt32(Buf);
		if (lCurve[Index].CurveType == SpTypeCurve)
			Status = SpCurveToPublic(Buf, &lCurve[Index].Curve);
		else 
			Status = SpParaCurveDataToPublic(Buf, &lCurve[Index].ParaCurve);
		while (((INTPTR)*Buf & 3) != 0) 
			(*Buf)++;	/* skip pad bytes */

	}

	return Status;

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *      Get CLUT Size
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      July 5, 2001
 *------------------------------------------------------------------*/
static KpUInt32_t GetCLUTSize (SpCLUT_t		*CLut,
			KpUInt32_t	InputChannels,
			KpUInt32_t	OutputChannels)
{
	KpUInt32_t	CLutSize, Index;

	
/*
CLutSize = InputChannels * OutputChannels;
*/

	CLutSize =  OutputChannels;
	for (Index = 0; Index < InputChannels; Index++)
	{
		CLutSize *= CLut->NumGridPt[Index];
	}
	CLutSize *= CLut->Precision;

	return CLutSize;
}
	

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get CLUT
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpGetABCLut (char	FAR * KPHUGE *Buf,
				SpCLUT_t	* FAR *CLut,
				KpUInt32_t	InputChan,
				KpUInt32_t	OutputChan)
{
	KpUInt32_t	Index, CLutSize;
	char		*btptr;
	SpCLUT_t	*lCLut;

	btptr = *Buf;
	lCLut = *CLut;
	for (Index = 0; Index < 16; Index++)
	{
		lCLut->NumGridPt[Index] = *btptr++;  /* Need code */
	}
	lCLut->Precision = *btptr;

	CLutSize = GetCLUTSize(lCLut, InputChan, OutputChan);

	/* Move past data gotten and the 3 reserved bytes */
	*Buf += CLUT_HEADER;

	lCLut->L.Lut8 = (unsigned char *)SpMalloc(CLutSize);
	if (lCLut->L.Lut8 == NULL)
		return SpStatMemory;

	if (CLutSize == 1)
		SpGetBytes (Buf, lCLut->L.Lut8, CLutSize);	/* 8-bit CLut Data */
	else
		SpGetUInt16s (Buf, lCLut->L.Lut16, CLutSize/2);

	/* Create and Set the CLUT buffer */
	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get Matrix
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpGetABMatrix (char	FAR * KPHUGE *Buf,
				KpF15d16_t	* FAR *Matrix)
{
KpF15d16_t	*lMatrix;

	lMatrix = *Matrix;

	SpGetF15d16 (Buf, lMatrix, 12);
	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put Curve 
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	July 5, 2001
 *------------------------------------------------------------------*/
static void SpPutABCLut(char * FAR *Buf, SpCLUT_t *CLUT, 
			KpUInt32_t  InputChannels, 
			KpUInt32_t  OutputChannels)
{
	KpUInt32_t	Index, CLutSize;
	char *BufPtr;

	BufPtr = *Buf;
	for (Index = 0; Index < InputChannels; Index++)
		*BufPtr++ = CLUT->NumGridPt[Index];

	for (Index = 15; Index >= InputChannels; Index--)
		*BufPtr++ = 0;

	*BufPtr++ = CLUT->Precision;
	*BufPtr++ = 0;
	*BufPtr++ = 0;  /* Clear out the reserved */
	*BufPtr++ = 0;

	CLutSize = GetCLUTSize(CLUT, InputChannels, OutputChannels);
	if (CLUT->Precision == 1)	/* 8-bit grid data */
		SpPutBytes(&BufPtr, CLutSize, CLUT->L.Lut8);
	else 
		SpPutUInt16s(&BufPtr, CLUT->L.Lut16, CLutSize/2);

	while ( ((INTPTR)BufPtr & 3) != 0)	/* pad to even 4 byte boundary */
		*BufPtr++ = 0;
	*Buf = BufPtr;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Put Curve 
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	July 5, 2001
 *------------------------------------------------------------------*/
static void SpPutABCurve (char * FAR *Buf, SpLutCurve_t	*Curve,
			KpUInt32_t NumChan)
{
	KpUInt32_t	Limit;
	KpUInt16_t	*UInt16Ptr;
	KpUInt32_t	Chan;
	SpLutCurve_t	*lCurve;

	for (Chan = 0; Chan < NumChan; Chan++)
	{
		lCurve = &Curve[Chan];
		if (lCurve->CurveType == SpTypeCurve)
		{
        		SpPutUInt32 (Buf, SpTypeCurve);
        		SpPutUInt32 (Buf, 0L);
			Limit = lCurve->Curve.Count;

			SpPutUInt32 (Buf, Limit);
			UInt16Ptr = lCurve->Curve.Data;
			while (Limit--)
				SpPutUInt16 (Buf, *UInt16Ptr++);
		} else
		{
        		SpPutUInt32 (Buf, SpTypeParametricCurve);
        		SpPutUInt32 (Buf, 0L);

			SpPutUInt16 (Buf, lCurve->ParaCurve.FuncType);
			SpPutUInt16 (Buf, 0); /* Add reserve */

			if (lCurve->ParaCurve.FuncType == 0)
				SpPutF15d16 (Buf, lCurve->ParaCurve.Parameters, 1);
			else if (lCurve->ParaCurve.FuncType == 1)
				SpPutF15d16 (Buf, lCurve->ParaCurve.Parameters, 3);
			else if (lCurve->ParaCurve.FuncType == 2)
				SpPutF15d16 (Buf, lCurve->ParaCurve.Parameters, 4);
			else if (lCurve->ParaCurve.FuncType == 3)
				SpPutF15d16 (Buf, lCurve->ParaCurve.Parameters, 5);
			else if (lCurve->ParaCurve.FuncType == 4)
				SpPutF15d16 (Buf, lCurve->ParaCurve.Parameters, 7);

		}
		while (((INTPTR)*Buf & 3) != 0) {
			*(*Buf)++ = 0;	/* add pad bytes */
		}

	}
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get Curve Size 
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	July 2, 2001
 *------------------------------------------------------------------*/
static KpUInt32_t GetCurveSize (
				SpLutCurve_t	*Curve)
{
	KpUInt32_t	Count = 0;

	if (Curve->CurveType == SpTypeCurve)
	{
		Count = sizeof( KpUInt32_t) + (Curve->Curve.Count * sizeof (KpUInt16_t)) + 
			8; /* 8 for sig and reserved */
	}
	else 
	{
		Count = 16; /* sig + function type + 1 parameter */
		if (Curve->ParaCurve.FuncType > 0)
			/* up to 6 parameters here */
			Count += (1 + Curve->ParaCurve.FuncType) * sizeof(KpF15d16_t);
		if (Curve->ParaCurve.FuncType == 4)
			Count += sizeof(KpF15d16_t); /* need 7 parameters here */
	}
	/* All curves in mBA and mAB type tags must start on even 4 byte boundary so
	   we need to make the count an even multiple of 4. */
	Count += 3;
	Count &= ~3;

	return Count;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lutAB From public format.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpLutABFromPublic (
				SpLut_t	FAR *Lut,
				void	KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize)
{
	SpLutAB_t	FAR *LutAB;
	KpUInt32_t	Size, Boffset, Matoffset, Moffset, CLoffset, Aoffset, CLsize;
	char		*Buf;
	KpUInt32_t	Chan;

	Boffset = Matoffset = Moffset = CLoffset = Aoffset = 0;
	LutAB = &Lut->L.LutAB;

/****************************/
/*  Get Size needed for tag */
/****************************/
	Size = 32;
	if (LutAB->Bcurve != NULL)
	{
		Boffset = Size;
		for (Chan = 0; Chan < LutAB->OutputChannels; Chan++)
			Size += GetCurveSize(&LutAB->Bcurve[Chan]);
	}
	if (LutAB->Matrix != NULL)
	{
		Matoffset = Size;
		Size += 48;
	}
	if (LutAB->Mcurve != NULL)
	{
		Moffset = Size;
		for (Chan = 0; Chan < LutAB->OutputChannels; Chan++)
			Size += GetCurveSize(&LutAB->Mcurve[Chan]);
	}
	if (LutAB->CLUT   != NULL)
	{
		CLoffset = Size;
		CLsize = GetCLUTSize(LutAB->CLUT, LutAB->InputChannels,
				LutAB->OutputChannels);
		Size += CLsize + CLUT_HEADER;
		while ((CLsize & 3) != 0) {
			Size++;	/* increment size to account for pad bytes */
			CLsize ++;
		}
	}
	if (LutAB->Acurve != NULL)
	{
		Aoffset = Size;
		for (Chan = 0; Chan < LutAB->InputChannels; Chan++)
			Size += GetCurveSize(&LutAB->Acurve[Chan]);
	}


	Buf = SpMalloc(Size);
        if (NULL == Buf)
                return SpStatMemory;

/* fill the buffer */
		*Buffer = Buf;
		*BufferSize = Size;
        SpPutUInt32 (&Buf, SpTypeLutAB);
        SpPutUInt32 (&Buf, 0L);
        *Buf++ = LutAB->InputChannels;
        *Buf++ = LutAB->OutputChannels;
	*Buf++ = 0;
	*Buf++ = 0; /* Clear padded bytes */

	SpPutUInt32 (&Buf, Boffset);
	SpPutUInt32 (&Buf, Matoffset);
	SpPutUInt32 (&Buf, Moffset);
	SpPutUInt32 (&Buf, CLoffset);
	SpPutUInt32 (&Buf, Aoffset);

	if (Boffset != 0)
		SpPutABCurve(&Buf, LutAB->Bcurve, 
			(KpUInt32_t)LutAB->OutputChannels);
	if (Matoffset != 0)
        	SpPutF15d16 (&Buf, LutAB->Matrix, 12);
	if (Moffset != 0)
		SpPutABCurve(&Buf, LutAB->Mcurve, 
			(KpUInt32_t)LutAB->OutputChannels);
	if (CLoffset != 0)
		SpPutABCLut(&Buf, LutAB->CLUT, 
			(KpUInt32_t)LutAB->InputChannels, 
			(KpUInt32_t)LutAB->OutputChannels);
	if (Aoffset != 0)
		SpPutABCurve(&Buf, LutAB->Acurve, 
			(KpUInt32_t)LutAB->InputChannels);

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lutBA from public format.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpLutBAFromPublic (
				SpLut_t	FAR *Lut,
				void	KPHUGE * FAR *Buffer,
				KpUInt32_t	FAR *BufferSize)
{
	SpLutAB_t	FAR *LutBA;
	KpUInt32_t	Size, Boffset, Matoffset, Moffset, CLoffset, Aoffset, CLsize;
	char		*Buf;
	KpUInt32_t	Chan;

	Boffset = Matoffset = Moffset = CLoffset = Aoffset = 0;
	LutBA = &Lut->L.LutAB;

/****************************/
/*  Get Size needed for tag */
/****************************/
	Size = 32;
	if (LutBA->Bcurve != NULL)
	{
		Boffset = Size;
		for (Chan = 0; Chan < LutBA->InputChannels; Chan++)
			Size += GetCurveSize(&LutBA->Bcurve[Chan]);
	}
	if (LutBA->Matrix != NULL)
	{
		Matoffset = Size;
		Size +=  3 * 4 * sizeof(KpF15d16_t);
	}
	if (LutBA->Mcurve != NULL)
	{
		Moffset = Size;
		for (Chan = 0; Chan < LutBA->InputChannels; Chan++)
			Size += GetCurveSize(&LutBA->Mcurve[Chan]);
	}
	if (LutBA->CLUT   != NULL)
	{
		CLoffset = Size;
		CLsize = GetCLUTSize(LutBA->CLUT, LutBA->InputChannels,
				LutBA->OutputChannels);
		Size += CLUT_HEADER + CLsize;
		while ((CLsize & 3) != 0) {
			Size++;	/* increment size to account for pad bytes */
			CLsize ++;
		}
	}
	if (LutBA->Acurve != NULL)
	{
		Aoffset = Size;
		for (Chan = 0; Chan < LutBA->OutputChannels; Chan++)
			Size += GetCurveSize(&LutBA->Acurve[Chan]);
	}


	Buf = SpMalloc(Size);
	*Buffer = Buf;
	*BufferSize = Size;
        if (NULL == Buf)
                return SpStatMemory;

/* fill the buffer */
        SpPutUInt32 (&Buf, SpTypeLutBA);
        SpPutUInt32 (&Buf, 0L);
        *Buf++ = LutBA->InputChannels;
        *Buf++ = LutBA->OutputChannels;
	*Buf++ = 0;
	*Buf++ = 0; /* Clear padded bytes */

	SpPutUInt32 (&Buf, Boffset);
	SpPutUInt32 (&Buf, Matoffset);
	SpPutUInt32 (&Buf, Moffset);
	SpPutUInt32 (&Buf, CLoffset);
	SpPutUInt32 (&Buf, Aoffset);

	if (Boffset != 0)
		SpPutABCurve(&Buf, LutBA->Bcurve, 
			(KpUInt32_t)LutBA->InputChannels);
	if (Matoffset != 0)
        	SpPutF15d16 (&Buf, LutBA->Matrix, 12);
	if (Moffset != 0)
		SpPutABCurve(&Buf, LutBA->Mcurve, 
			(KpUInt32_t)LutBA->InputChannels);
	if (CLoffset != 0)
		SpPutABCLut(&Buf, LutBA->CLUT, 
			(KpUInt32_t)LutBA->InputChannels, 
			(KpUInt32_t)LutBA->OutputChannels);
	if (Aoffset != 0)
		SpPutABCurve(&Buf, LutBA->Acurve, 
			(KpUInt32_t)LutBA->OutputChannels);

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create empty AB Lut.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	Feb 26, 2002
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpLutABCreate (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt32_t	BCurve,
				KpUInt32_t	Matrix,
				KpUInt32_t	MCurve,
				KpUInt32_t	CLUT,
				KpUInt32_t	ACurve,
				SpLut_t		FAR *Lut)
{
	SpLutAB_t		FAR *LutAB;

/* validate parameters */
	if ((InChannels < 1) || (16 < InChannels))
		return SpStatOutOfRange;

	if ((OutChannels < 1) || (16 < OutChannels))
		return SpStatOutOfRange;

/* initialize all structure members */
	Lut->LutType = SpTypeLutAB;
	LutAB = &Lut->L.LutAB;
	LutAB->InputChannels = (char) InChannels;
	LutAB->OutputChannels = (char) OutChannels;

/* Initialize incase zero offset so not used */
	LutAB->Bcurve = NULL;
	LutAB->Matrix = NULL;
	LutAB->Mcurve = NULL;
	LutAB->CLUT   = NULL;
	LutAB->Acurve = NULL;

	if (BCurve != 0)
	{
		LutAB->Bcurve = (SpLutCurve_t *)SpMalloc(OutChannels * sizeof(SpLutCurve_t));
		if (LutAB->Bcurve == NULL)
			return SpStatMemory;
	}
	if (Matrix != 0)
	{
		LutAB->Matrix = (KpF15d16_t *)SpMalloc(12 * sizeof(KpF15d16_t));
		if (LutAB->Matrix == NULL)
		{
			if (BCurve != 0)
				SpFree(LutAB->Bcurve);
			return SpStatMemory;
		}
		SpInitMatrix12 (LutAB->Matrix);
	}
	if (MCurve != 0)
	{
		LutAB->Mcurve = (SpLutCurve_t *)SpMalloc(OutChannels * sizeof(SpLutCurve_t));
		if (LutAB->Mcurve == NULL)
		{
			if (BCurve != 0)
				SpFree(LutAB->Bcurve);
			if (Matrix != 0)
				SpFree(LutAB->Matrix);
			return SpStatMemory;
		}
	}
	if (CLUT != 0)
	{
		LutAB->CLUT = (SpCLUT_t *)SpMalloc(sizeof(SpCLUT_t));
		if (LutAB->CLUT == NULL)
		{
			if (BCurve != 0)
				SpFree(LutAB->Bcurve);
			if (Matrix != 0)
				SpFree(LutAB->Matrix);
			if (MCurve != 0)
				SpFree(LutAB->Mcurve);
			return SpStatMemory;
		}
	}
	if (ACurve != 0)
	{
		LutAB->Acurve = (SpLutCurve_t *)SpMalloc(InChannels * sizeof(SpLutCurve_t));
		if (LutAB->Acurve == NULL)
		{
			if (BCurve != 0)
				SpFree(LutAB->Bcurve);
			if (Matrix != 0)
				SpFree(LutAB->Matrix);
			if (MCurve != 0)
				SpFree(LutAB->Mcurve);
			if (CLUT != 0)
				SpFree(LutAB->CLUT);
			return SpStatMemory;
		}
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create empty BA Lut.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	Feb 26, 2002
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpLutBACreate (
				KpUInt16_t	InChannels,
				KpUInt16_t	OutChannels,
				KpUInt32_t	BCurve,
				KpUInt32_t	Matrix,
				KpUInt32_t	MCurve,
				KpUInt32_t	CLUT,
				KpUInt32_t	ACurve,
				SpLut_t		FAR *Lut)
{
	SpLutAB_t		FAR *LutBA;

/* validate parameters */
	if ((InChannels < 1) || (16 < InChannels))
		return SpStatOutOfRange;

	if ((OutChannels < 1) || (16 < OutChannels))
		return SpStatOutOfRange;

/* initialize all structure members */
	Lut->LutType = SpTypeLutBA;
	LutBA = &Lut->L.LutAB;
	LutBA->InputChannels = (char) InChannels;
	LutBA->OutputChannels = (char) OutChannels;

/* Initialize incase zero offset so not used */
	LutBA->Bcurve = NULL;
	LutBA->Matrix = NULL;
	LutBA->Mcurve = NULL;
	LutBA->CLUT   = NULL;
	LutBA->Acurve = NULL;

	if (BCurve != 0)
	{
		LutBA->Bcurve = (SpLutCurve_t *)SpMalloc(InChannels * sizeof(SpLutCurve_t));
		if (LutBA->Bcurve == NULL)
			return SpStatMemory;
	}
	if (Matrix != 0)
	{
		LutBA->Matrix = (KpF15d16_t *)SpMalloc(12 * sizeof(KpF15d16_t));
		if (LutBA->Matrix == NULL)
		{
			if (BCurve != 0)
				SpFree(LutBA->Bcurve);
			return SpStatMemory;
		}
		SpInitMatrix12 (LutBA->Matrix);
	}
	if (MCurve != 0)
	{
		LutBA->Mcurve = (SpLutCurve_t *)SpMalloc(InChannels * sizeof(SpLutCurve_t));
		if (LutBA->Mcurve == NULL)
		{
			if (BCurve != 0)
				SpFree(LutBA->Bcurve);
			if (Matrix != 0)
				SpFree(LutBA->Matrix);
			return SpStatMemory;
		}
	}
	if (CLUT != 0)
	{
		LutBA->CLUT = (SpCLUT_t *)SpMalloc(sizeof(SpCLUT_t));
		if (LutBA->CLUT == NULL)
		{
			if (BCurve != 0)
				SpFree(LutBA->Bcurve);
			if (Matrix != 0)
				SpFree(LutBA->Matrix);
			if (MCurve != 0)
				SpFree(LutBA->Mcurve);
			return SpStatMemory;
		}
	}
	if (ACurve != 0)
	{
		LutBA->Acurve = (SpLutCurve_t *)SpMalloc(OutChannels * sizeof(SpLutCurve_t));
		if (LutBA->Acurve == NULL)
		{
			if (BCurve != 0)
				SpFree(LutBA->Bcurve);
			if (Matrix != 0)
				SpFree(LutBA->Matrix);
			if (MCurve != 0)
				SpFree(LutBA->Mcurve);
			if (CLUT != 0)
				SpFree(LutBA->CLUT);
			return SpStatMemory;
		}
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lutAB to public format.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpLutABToPublic (
				char	KPHUGE *Buf,
				SpLut_t	FAR *Lut)
{
	KpUInt32_t	Index, ThisOffset;
	SpLutAB_t	FAR *LutAB;
	KpUInt32_t	Boffset, Matoffset, Moffset, CLoffset, Aoffset;
	KpUInt16_t	InChannels, OutChannels;
	char		*BufStart;
	SpStatus_t	Status;

	BufStart = Buf - 8; /* Point to sig and reserved */
	LutAB = &Lut->L.LutAB;

/* get fixed size elements */
	InChannels  = *Buf++;
	OutChannels = *Buf++;
	Buf++;Buf++; /* Get Past Padding */

	Boffset = SpGetUInt32(&Buf);
	Matoffset = SpGetUInt32(&Buf);
	Moffset = SpGetUInt32(&Buf);
	CLoffset = SpGetUInt32(&Buf);
	Aoffset = SpGetUInt32(&Buf);

	Status = SpLutABCreate(InChannels, OutChannels, Boffset, Matoffset,
				Moffset, CLoffset, Aoffset, Lut);
	if (Status != SpStatSuccess)
		return Status;

	/* Get the correct part, and adjust for starting on double word boundaries */
	for (Index = 0; (Index < 5 && Status == SpStatSuccess); Index++)
	{
		ThisOffset = (KpUInt32_t)(Buf - BufStart);
		if (ThisOffset == Boffset)
			Status = SpGetABCurve(&Buf, &LutAB->Bcurve, 
				(KpUInt32_t)OutChannels);
		if (ThisOffset == CLoffset)
			Status = SpGetABCLut(&Buf, &LutAB->CLUT, 
				(KpUInt32_t)InChannels, 
				(KpUInt32_t)OutChannels);
		if (ThisOffset == Matoffset)
			Status = SpGetABMatrix(&Buf, &LutAB->Matrix);
		if (ThisOffset == Moffset)
			Status = SpGetABCurve(&Buf, &LutAB->Mcurve, 
				(KpUInt32_t)OutChannels);
		if (ThisOffset == Aoffset)
			Status = SpGetABCurve(&Buf, &LutAB->Acurve, 
				(KpUInt32_t)InChannels);
	}
	return Status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lutBA to public format.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	April 27, 2001
 *------------------------------------------------------------------*/
static SpStatus_t SpLutBAToPublic (
				char	KPHUGE *Buf,
				SpLut_t	FAR *Lut)
{
	KpUInt32_t	Index, ThisOffset;
	SpLutAB_t	FAR *LutBA;
	KpUInt32_t	Boffset, Matoffset, Moffset, CLoffset, Aoffset;
	KpUInt16_t	InChannels, OutChannels;
	char		*BufStart;
	SpStatus_t	Status;

	BufStart = Buf - 8; /* Point to sig and reserved */
	LutBA = &Lut->L.LutAB;

/* get fixed size elements */
	InChannels  = *Buf++;
	OutChannels = *Buf++;
	Buf++;Buf++; /* Get Past Padding */

	Boffset = SpGetUInt32(&Buf);
	Matoffset = SpGetUInt32(&Buf);
	Moffset = SpGetUInt32(&Buf);
	CLoffset = SpGetUInt32(&Buf);
	Aoffset = SpGetUInt32(&Buf);

	Status = SpLutBACreate(InChannels, OutChannels, Boffset, Matoffset,
				Moffset, CLoffset, Aoffset, Lut);
	if (Status != SpStatSuccess)
		return Status;

	/* Get the correct part, and adjust for starting on double word boundaries */
	for (Index = 0; (Index < 5 && Status == SpStatSuccess); Index++)
	{
		ThisOffset = (KpUInt32_t)(Buf - BufStart ); /* Make up for Sig and Reserved */
		if (ThisOffset == Boffset)
			Status = SpGetABCurve(&Buf, &LutBA->Bcurve, 
				(KpUInt32_t)InChannels);
		if (ThisOffset == CLoffset)
			Status = SpGetABCLut(&Buf, &LutBA->CLUT, 
				(KpUInt32_t)InChannels, 
				(KpUInt32_t)OutChannels);
		if (ThisOffset == Matoffset)
			Status = SpGetABMatrix(&Buf, &LutBA->Matrix);
		if (ThisOffset == Moffset)
			Status = SpGetABCurve(&Buf, &LutBA->Mcurve, 
				(KpUInt32_t)InChannels);
		if (ThisOffset == Aoffset)
			Status = SpGetABCurve(&Buf, &LutBA->Acurve, 
				(KpUInt32_t)OutChannels);
	}
	return Status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lut to public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpLutToPublic (
				char	KPHUGE *Buf,
				SpLut_t	FAR *Lut)
{

/* get type of lut */
	switch (Lut->LutType) {
	case SpTypeLut8:
		return SpLut8ToPublic (Buf, Lut);

	case SpTypeLut16:
		return SpLut16ToPublic (Buf, Lut);

	case SpTypeLutAB:
		return SpLutABToPublic (Buf, Lut);

	case SpTypeLutBA:
		return SpLutBAToPublic (Buf, Lut);
	}

	return SpStatBadLutType;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert lut from public format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 5, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpLutFromPublic (
				SpLut_t		FAR *Lut,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer)
{

/* determine type of lut */
	switch (Lut->LutType) {
	case SpTypeLut8:
		return SpLut8FromPublic (Lut, Buffer, BufferSize);

	case SpTypeLut16:
		return SpLut16FromPublic (Lut, Buffer, BufferSize);

	case SpTypeLutAB:
		return SpLutABFromPublic (Lut, Buffer, BufferSize);

	case SpTypeLutBA:
		return SpLutBAFromPublic (Lut, Buffer, BufferSize);

	}

	return SpStatBadLutType;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free a lut.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	April 21, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpLutFree (
				SpLut_t		FAR *Lut)
{
int	i;

	switch (Lut->LutType) {
	case SpTypeLut8:
		SpFree (Lut->L.Lut8.InputTable);
		SpFree (Lut->L.Lut8.CLUT);
		SpFree (Lut->L.Lut8.OutputTable);
		break;

	case SpTypeLut16:
		SpFree (Lut->L.Lut16.InputTable);
		SpFree (Lut->L.Lut16.CLUT);
		SpFree (Lut->L.Lut16.OutputTable);
		break;

	case SpTypeLutAB:
		if (Lut->L.LutAB.Bcurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.OutputChannels; i++)
			{
				if (Lut->L.LutAB.Bcurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Bcurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Bcurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Bcurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Bcurve);
		}
		if (Lut->L.LutAB.Matrix != NULL)
			SpFree(Lut->L.LutAB.Matrix);
		if (Lut->L.LutAB.Mcurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.OutputChannels; i++)
			{
				if (Lut->L.LutAB.Mcurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Mcurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Mcurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Mcurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Mcurve);
		}
		if (Lut->L.LutAB.CLUT != NULL)
		{
			if (Lut->L.LutAB.CLUT->Precision == 1)
				SpFree(Lut->L.LutAB.CLUT->L.Lut8);
			else if (Lut->L.LutAB.CLUT->Precision == 2)
				SpFree(Lut->L.LutAB.CLUT->L.Lut16);
			SpFree(Lut->L.LutAB.CLUT);
		}
		if (Lut->L.LutAB.Acurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.InputChannels; i++)
			{
				if (Lut->L.LutAB.Acurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Acurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Acurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Acurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Acurve);
		}

		break;

	case SpTypeLutBA:
		if (Lut->L.LutAB.Bcurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.InputChannels; i++)
			{
				if (Lut->L.LutAB.Bcurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Bcurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Bcurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Bcurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Bcurve);
		}
		if (Lut->L.LutAB.Matrix != NULL)
			SpFree(Lut->L.LutAB.Matrix);
		if (Lut->L.LutAB.Mcurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.InputChannels; i++)
			{
				if (Lut->L.LutAB.Mcurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Mcurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Mcurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Mcurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Mcurve);
		}
		if (Lut->L.LutAB.CLUT != NULL)
		{
			if (Lut->L.LutAB.CLUT->Precision == 1)
				SpFree(Lut->L.LutAB.CLUT->L.Lut8);
			else if (Lut->L.LutAB.CLUT->Precision == 2)
				SpFree(Lut->L.LutAB.CLUT->L.Lut16);
			SpFree(Lut->L.LutAB.CLUT);
		}
		if (Lut->L.LutAB.Acurve != NULL)
		{
			for (i = 0; i < (int)Lut->L.LutAB.OutputChannels; i++)
			{
				if (Lut->L.LutAB.Acurve[i].CurveType == SpTypeParametricCurve)
					SpFree(Lut->L.LutAB.Acurve[i].ParaCurve.Parameters);
				else
					if (Lut->L.LutAB.Acurve[i].Curve.Data != NULL)
						SpFree(Lut->L.LutAB.Acurve[i].Curve.Data);
			}
			SpFree (Lut->L.LutAB.Acurve);
		}

		break;

	default:
		return SpStatBadTagType;
	}

	return SpStatSuccess;
}



     
     
