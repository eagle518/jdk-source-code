/*
 * @(#)spxfromr.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/* @(#)spxfromr.c	1.40 99/01/20
	Contains:	This module contains a conversion function to convert
				Color Processor Transforms (RefNums) to Standard
				profile transforms (SpXform_t).

				Created by lsh, Feb. 5, 1994

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1994-2003
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "sprof-pr.h"
#include <stdio.h>


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Construct Xform given a PT reference number.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 25, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformFromPTRefNumEx (
				SpConnectType_t	ConnectType,
				SpParadigm_t	ParadigmType,
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform)
{
	KpInt32_t	SpaceIn, SpaceOut;
	KpInt32_t	SenseIn, SenseOut;
	KpInt32_t	Class, Render;
	int			Index;
	KpInt32_t	FailXform;
	SpStatus_t	Status;
	PTRefNum_t	CvrtInRefNum, CvrtOutRefNum;
	PTRefNum_t	RefNumList [3];
	PTRefNum_t	NewRefNum;
	PTErr_t		InvertStat;

	Class = SpGetKcmAttrInt (*RefNum, KCM_CLASS);
	SpaceIn = SpGetKcmAttrInt (*RefNum, KCM_SPACE_IN);
	SpaceOut = SpGetKcmAttrInt (*RefNum, KCM_SPACE_OUT);
	SenseIn = SpGetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_IN);
	SenseOut = SpGetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_OUT);

/* setup to fix color spaces */
	Index = 0;
	Status = SpStatSuccess;

	KpEnterCriticalSection (&SpCacheCritFlag);

/* setup to fix input color space */
	if (KCM_RCS == SpaceIn) {
		Render = SpGetKcmAttrInt (*RefNum, KCM_COMPRESSION_OUT);
		if (Render == KCM_UNKNOWN) Render = KCM_PERCEPTUAL;
		Status = SpXformBuildCnvrt (KPTRUE, Render, ConnectType, ParadigmType, &CvrtInRefNum);
		if (SpStatSuccess == Status) {
			RefNumList [Index++] = CvrtInRefNum;
			RefNumList [Index++] = *RefNum;
		}
	}
	else
		RefNumList [Index++] = *RefNum;

/* setup to fix output color space */
	if (KCM_RCS == SpaceOut) {
		Render = KCM_PERCEPTUAL;
		if (SpStatSuccess == Status)
			Status = SpXformBuildCnvrt (KPFALSE, Render, 
				ConnectType, ParadigmType, &CvrtOutRefNum);

		if (SpStatSuccess == Status)
			RefNumList [Index++] = CvrtOutRefNum;
	}

/* fix the color spaces */
	if ((SpStatSuccess == Status) && (1 != Index)) {
		Status = SpConnectSequenceImp (ConnectType, Index, RefNumList,
									&NewRefNum, &FailXform, NULL, NULL);
		PTCheckOut (*RefNum);
		*RefNum = NewRefNum;
	}

	KpLeaveCriticalSection (&SpCacheCritFlag);

	if (SpStatSuccess != Status)
		return Status;

/* Invert input, if negative */
	if ((KCM_RCS != SpaceIn) && (KCM_NEGATIVE == SenseIn)) {
		InvertStat = PTInvert (*RefNum, KCM_MEDIUM_SENSE_IN);
		if (InvertStat != KCP_SUCCESS) {
			PTCheckOut (*RefNum);
			return SpStatusFromPTErr(InvertStat);
		}
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_IN, KCM_POSITIVE);
		if (SpStatSuccess != Status) {
			PTCheckOut (*RefNum);
			return Status;
		}
	}

/* Invert output, if negative */
	if ((KCM_RCS != SpaceOut) && (KCM_NEGATIVE == SenseOut)) {
		InvertStat = PTInvert (*RefNum, KCM_MEDIUM_SENSE_OUT);
		if (InvertStat != KCP_SUCCESS) {
			PTCheckOut (*RefNum);
			return SpStatusFromPTErr(InvertStat);
		}
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_OUT, KCM_POSITIVE);
		if (SpStatSuccess != Status) {
			PTCheckOut (*RefNum);
			return Status;
		}
	}

	Status = SpSetKcmAttrInt (*RefNum, KCM_CLASS, Class);
	if (SpStatSuccess != Status) {
		PTCheckOut (*RefNum);
		return Status;
	}

	Status = SpXformFromPTRefNumImp (*RefNum, Xform);
	if (SpStatSuccess != Status) 
		PTCheckOut (*RefNum);

	*RefNum = 0;

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create an SpXform_t from a data block containing a PT.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	February 6, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformCreateFromDataEx (
				SpConnectType_t	ConnectType,
				KpInt32_t	Size,
				KpLargeBuffer_t	Data,
				SpXform_t	FAR *Xform)
{
	SpStatus_t	Status;
	PTRefNum_t	RefNum;


	*Xform = NULL;

	Status = SpXformLoadImp (Data, Size, 
				 NO_DT_ICC_TYPE, SpSigNone, SpSigNone, &RefNum);
	if (SpStatSuccess != Status)
		return Status;

	Status = SpXformFromPTRefNumEx (ConnectType, SpParadigmRel, &RefNum, Xform);

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformCreateFromDataEx\n *Xform %x\n",*Xform);
	kcpDiagLog (string); }
	#endif

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create an SpXform_t from a data block containing a PT using the
 *	default grid size.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	February 6, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformCreateFromData (
				KpInt32_t	Size,
				KpLargeBuffer_t	Data,
				SpXform_t	FAR *Xform)
{
	return SpXformCreateFromDataEx (
			SpGetConnectType (), 
			Size, Data, Xform);
}



