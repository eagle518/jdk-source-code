/*
 * @(#)sptagio.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module supplies services for new tags
			supported after August 99

				Created by doro, August 24, 1999

	Written by:	The Kodak CMS MS Solaris Team

	Copyright:	(C) 1993 - 1999 by Eastman Kodak Company, 
			all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sptagio.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/sptagio.c $
		$Revision: 3 $
		$Date: 7/06/00 3:51p $
		$Author: Msm $

	SCCS Revision:
		@(#)sptagio.c	1.1 8/24/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993 - 1999               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"


static void SpPutResp16 (char KPHUGE * FAR *Buf, 
			SpResponse16_t *Resp16)
{

	SpPutUInt16 (Buf, Resp16->Interval);
	SpPutUInt16 (Buf, 0);
	SpPutUInt32 (Buf, Resp16->MeasVal);

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the Public structures and populates the tag data
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpRespFromPublic (	SpResCurveTag_t	FAR *ResCurve,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer)
{
	char	KPHUGE *Buf;
	char	KPHUGE *StartBuf;
	int	Ind, i, j, k, TagSize;
	int	MeasCount, NumChan;
	char	*Offsets;
	KpUInt32_t	OffsetVal;

	/* Signature, Reserved, NumChan, Count */
	TagSize = 12;

	MeasCount = ResCurve->MeasCount;
	NumChan   = ResCurve->NumChan;

	/* Room for the offsets */
	TagSize += MeasCount * sizeof(KpInt32_t);

	/* Now add in the known size per MeasCount parts */
	TagSize += MeasCount * 4; /* This covers the Signatures */
	TagSize += MeasCount * NumChan * 16; /* Counts and XYZs */

	for (i = 0; i < MeasCount; i++)
	{
		for (j = 0; j < NumChan; j++)
		{
			/* room for each channel's response values */
			TagSize += ResCurve->ResCurves[i].RespCount[j] * 8;
		}
	}

	*BufferSize = TagSize;

	Buf = SpMalloc(TagSize);

	if (NULL == Buf)
		return SpStatMemory;

	StartBuf = Buf;
	*Buffer = Buf;

	SpPutUInt32 (&Buf, (KpUInt32_t) SpTypeResponseCurve);
	SpPutUInt32 (&Buf, 0L);

	SpPutUInt16 (&Buf, (KpUInt16_t)ResCurve->NumChan);
	SpPutUInt16 (&Buf, (KpUInt16_t)ResCurve->MeasCount);

	/* Now set up to store offsets */
	Offsets = Buf;

	/* Now move Buf to beyond Offsets to fill in the rest */
	Buf += MeasCount * 4;

	for (i = 0; i < MeasCount; i++)
	{
		OffsetVal = (int)(Buf - StartBuf);
		SpPutUInt32(&Offsets, OffsetVal);

		SpPutUInt32 (&Buf, ResCurve->ResCurves[i].MeasSig);
		for (j = 0; j < NumChan; j++)
		{
			SpPutUInt32 (&Buf, 
				ResCurve->ResCurves[i].RespCount[j]);
		}

		for (j = 0; j < NumChan; j++)
		{
			SpPutF15d16XYZ (&Buf, 
				&ResCurve->ResCurves[i].MeasArray[j]);
		}
		Ind = 0;
		for (j = 0; j < NumChan; j++)
		{
			for (k = 0; 
				k < ResCurve->ResCurves[i].RespCount[j]; 
				k++)
			{
				SpPutResp16 (&Buf, 
					&ResCurve->ResCurves[i].RespData[Ind++]);
			}
		}
	}

	return SpStatSuccess;
}

static void SpGetResp16(char 	KPHUGE	* FAR *buf, 
			SpResponse16_t	*Resp16,
			KpUInt32_t	Count)
{
	int	i;
	char	*ThisPtr;
	int	dummy;

	ThisPtr = *buf;

	for (i = 0; i < (int)Count; i++)
	{
		Resp16[i].Interval = SpGetUInt16(&ThisPtr);
		/* Move over Padding */
		dummy = SpGetUInt16(&ThisPtr);
		Resp16[i].MeasVal = SpGetUInt32(&ThisPtr);
	}
	*buf = ThisPtr;

}

static SpStatus_t SpRespCurveToPublic(
			int		ChanCount,
			char		KPHUGE *buf,
			SpResCurve_t	*Meas)
{
	int 	i, Ind;
	int	MeasSize;

	Meas->MeasSig = SpGetUInt32 (&buf);

	Meas->RespCount = SpMalloc(ChanCount * 4);
	if (Meas->RespCount == NULL)
		return SpStatMemory;

	Meas->MeasArray = SpMalloc(ChanCount * 12);
	if (Meas->MeasArray == NULL)
		return SpStatMemory;

	MeasSize = 0;
	for (i = 0; i < ChanCount; i++)
	{
		Meas->RespCount[i] = SpGetUInt32 (&buf);
		MeasSize += Meas->RespCount[i];
	}

	Meas->RespData = SpMalloc(MeasSize * sizeof(SpResponse16_t));
	if (Meas->RespData == NULL)
		return SpStatMemory;

	for (i = 0; i < ChanCount; i++)
		SpGetF15d16XYZ (&buf, &Meas->MeasArray[i]);

	Ind = 0;
	for (i = 0; i < ChanCount; i++)
	{
		SpGetResp16(&buf, &Meas->RespData[Ind],
			Meas->RespCount[i]);
		Ind += Meas->RespCount[i];
	}
	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the tag data and populates the Public structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpRespToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpResCurveTag_t	FAR *ResCurve)
{

	int	i, MeasCount, ChanCount;
	SpStatus_t	Status;

	ChanCount = SpGetUInt16 (&buf);
	MeasCount = SpGetUInt16 (&buf);

	/* Accounting for Type, Reserved, Number of Channels,
	   Measurement Count, Offset Array, Measurement Signatures,
	   Number of Measurements per channel for each measurement,
	   and Measurements for each channel for each Measurement.
	   There is nothing that says the number of measurements for
	   each channel (number of response arrays) cannot be 0 so
	   don't expect room for them */
	if ((int)BufferSize < (12 + MeasCount * (8 + ChanCount * 16)))
		return SpStatBadTagData;

	ResCurve->ResCurves = SpMalloc(MeasCount * sizeof(SpResCurve_t));
	if (ResCurve->ResCurves == NULL)
		return	SpStatMemory;

	ResCurve->NumChan = ChanCount;
	ResCurve->MeasCount = MeasCount;

	/* Jump over the offset array */
	buf += 4 * MeasCount;

	for (i = 0; i < MeasCount; i++)
	{
		Status = SpRespCurveToPublic(ChanCount, buf,
			&ResCurve->ResCurves[i]);

		if (Status != SpStatSuccess)
			return Status;
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the Public structures and populates the tag data
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpDevSetFromPublic (	SpDevStruct_t	FAR *DevSet,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer)
{
	char	KPHUGE *Buf;
	int	i, j, k, l, PlatCount, TagSize;
	SpPlatStruct_t	*Platforms;
	SpCombStruct_t	*Comb;
	SpSetStruct_t	*SetArray;
	SpUInt64_t	*ThisRes;
	KpUInt32_t	*ThisVal;

	/* Signature, Reserved, Count */
	TagSize = 12;
	PlatCount = DevSet->NumOfPlatforms;
	Platforms = DevSet->PlatformArray;

	for (i = 0; i < PlatCount; i++, Platforms++)
	{
		TagSize += Platforms->PlatformSize;
	}

	*BufferSize = TagSize;

	Buf = SpMalloc(*BufferSize);

	if (NULL == Buf)
		return SpStatMemory;

	*Buffer = Buf;

	/* Signature and Reserved */
	SpPutUInt32 (&Buf, (KpUInt32_t) SpTypeDevSettings);
	SpPutUInt32 (&Buf, 0L);

	/* Platform COunt */
	SpPutUInt32 (&Buf, PlatCount);

	Platforms = DevSet->PlatformArray;
	for (i = 0; i < PlatCount; i++, Platforms++)
	{
		SpPutUInt32 (&Buf, Platforms->PlatformId);
		SpPutUInt32 (&Buf, Platforms->PlatformSize);
		SpPutUInt32 (&Buf, Platforms->CombCount);

		for (j = 0, Comb = Platforms->CombArray;
		     j < Platforms->CombCount;
		     j++, Comb++)
		{
			SpPutUInt32 (&Buf, Comb->CombSize);
			SpPutUInt32 (&Buf, Comb->SetCount);

			for (k = 0, SetArray = Comb->SetArray;
			     k < Comb->SetCount;
			     k++, SetArray++)
			{
				SpPutUInt32 (&Buf, SetArray->SettingSig);
				SpPutUInt32 (&Buf, SetArray->SettingSize);
				SpPutUInt32 (&Buf, SetArray->numSettings);
				if (SetArray->SettingSize == 8)
				{
					for (l = 0, ThisRes = (SpUInt64_t *)SetArray->Setting;
					     l < SetArray->numSettings;
					     l++, ThisRes++)
					{
						SpPutUInt32 (&Buf, ThisRes->hi);
						SpPutUInt32 (&Buf, ThisRes->lo);
					}
				} else
				{
					for (l = 0, ThisVal = (KpUInt32_t *)SetArray->Setting;
					     l < SetArray->numSettings;
					     l++, ThisVal++)
					{
						SpPutUInt32 (&Buf, *ThisVal);
					}
				}
			}
		}
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the tag data and populates the Public structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
static SpStatus_t SpSetArrayToPublic (	char		KPHUGE *buf,
				SpSetStruct_t	FAR *SetArray)
{
	int	i, ValCount;
	SpUInt64_t	*Resolut;
	KpUInt32_t	*Media;

	SetArray->SettingSig = SpGetUInt32(&buf);
	SetArray->SettingSize = SpGetUInt32(&buf);
	ValCount = SpGetUInt32(&buf);
	SetArray->numSettings = ValCount;
	
	if (SetArray->SettingSize == 8)
	{
		Resolut = SpMalloc(ValCount * sizeof(SpSetStruct_t));
		if (Resolut == NULL)
			return  SpStatMemory;

		SetArray->Setting = (void *)Resolut;
		for (i = 0; i < ValCount; i++, Resolut++)
		{
			Resolut->hi = SpGetUInt32(&buf );
			Resolut->lo = SpGetUInt32(&buf );
		}
	} else
	{
		Media = SpMalloc(ValCount * sizeof(KpUInt32_t));
		if (Media == NULL)
			return  SpStatMemory;

		SetArray->Setting = (void *)Media;
		for (i = 0; i < ValCount; i++, Media++)
			*Media = SpGetUInt32(&buf );
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the tag data and populates the Public structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
static SpStatus_t SpCombsToPublic (	char		KPHUGE *buf,
				SpCombStruct_t	FAR *Comb)
{
	SpStatus_t	Status;
	int	i, SetCount;
	SpSetStruct_t	*SetArray;

	Comb->CombSize = SpGetUInt32(&buf);
	SetCount = SpGetUInt32(&buf);
	Comb->SetCount = SetCount;
	
	SetArray = SpMalloc(SetCount * sizeof(SpSetStruct_t));
	if (SetArray == NULL)
		return  SpStatMemory;

	Comb->SetArray = SetArray;
	for (i = 0; i < SetCount; i++, SetArray++)
	{
		Status = SpSetArrayToPublic(buf, SetArray);
		if (Status != SpStatSuccess)
			return Status;
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the tag data and populates the Public structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
static SpStatus_t SpPlatformToPublic (	char		KPHUGE *buf,
				SpPlatStruct_t	FAR *Platform)
{
	int	i, CombCount;
	SpStatus_t	Status;
	SpCombStruct_t	*Combs;

	Platform->PlatformId = SpGetUInt32(&buf);
	Platform->PlatformSize = SpGetUInt32(&buf);
	CombCount = SpGetUInt32(&buf);
	Platform->CombCount = CombCount;
	
	Combs = SpMalloc(CombCount * sizeof(SpCombStruct_t));
	if (Combs == NULL)
		return  SpStatMemory;

	Platform->CombArray = Combs;
	for (i = 0; i < CombCount; i++, Combs++)
	{
		Status = SpCombsToPublic(buf, Combs);
		if (Status != SpStatSuccess)
			return Status;
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the tag data and populates the Public structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpDevSetToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpDevStruct_t	FAR *DevSet)
{
	int	i, PlatCount;
	SpStatus_t	Status;
	SpPlatStruct_t	*Platforms;

	PlatCount = SpGetUInt32(&buf);
	
	DevSet->NumOfPlatforms = PlatCount;

	if (BufferSize < (PlatCount*sizeof(SpPlatStruct_t) + 32))
		return SpStatBadTagData;

	Platforms = SpMalloc(PlatCount * sizeof(SpPlatStruct_t));
	if (Platforms == NULL)
		return  SpStatMemory;

	DevSet->PlatformArray = Platforms;
	for (i = 0; i < PlatCount; i++, Platforms++)
	{
		Status = SpPlatformToPublic(buf, Platforms);
		if (Status != SpStatSuccess)
			return Status;
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the Public structures and populates the tag data
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpChromFromPublic (	SpChromaticity_t	FAR *Chrom,
				KpUInt32_t	FAR *BufferSize,
				void		KPHUGE * FAR *Buffer)
{
	char	KPHUGE *Buf;
	int	i;

	/* X and Y per channel + Sig and Reserved + Chan and Type */
	*BufferSize = Chrom->NumChan * 2 * 4 + 12;

	Buf = SpMalloc(*BufferSize);

	if (NULL == Buf)
		return SpStatMemory;

	*Buffer = Buf;
	SpPutUInt32 (&Buf, (KpUInt32_t) SpTypeChromaticity);
	SpPutUInt32 (&Buf, 0L);

	SpPutUInt16 (&Buf, (KpUInt16_t)Chrom->NumChan);
	SpPutUInt16 (&Buf, (KpUInt16_t)Chrom->ColorType);

	for (i = 0; i < Chrom->NumChan; i++)
	{
		/* This is really a 16Fixed16, but 15d16 puts it in
		   just fine */
		SpPutF15d16 (&Buf, &Chrom->Coords[i].x, 1);
		SpPutF15d16 (&Buf, &Chrom->Coords[i].y, 1);
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Takes the Public structures and populates the tag data
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
SpStatus_t SpChromToPublic (	KpUInt32_t	BufferSize,
				char		KPHUGE *buf,
				SpChromaticity_t	FAR *Chrom)
{
	int	i;

	if (BufferSize < 20)
		return SpStatBadTagData;

	Chrom->NumChan = SpGetUInt16 (&buf);

	if ((int)BufferSize < (Chrom->NumChan*8 + 12))
		return SpStatBadTagData;

	Chrom->Coords = SpMalloc(Chrom->NumChan * sizeof(SpChromCoord_t));
	if (Chrom->Coords == NULL)
		return SpStatMemory;

	Chrom->ColorType = SpGetUInt16 (&buf);

	/* Get15d16 moves 4 bytes */
	for (i = 0; i < Chrom->NumChan; i++)
	{
		SpGetF15d16(&buf, &Chrom->Coords[i].x, 1);
		SpGetF15d16(&buf, &Chrom->Coords[i].y, 1);
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Frees the Public Tag Structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
void SpRespFree (	SpResCurveTag_t	*pRespTag)
{

	int	i, MeasCount;

	MeasCount = pRespTag->MeasCount;

	for (i = 0; i < MeasCount; i++)
	{
		SpFree(pRespTag->ResCurves[i].RespData);
		SpFree(pRespTag->ResCurves[i].MeasArray);
		SpFree(pRespTag->ResCurves[i].RespCount);
	}
	SpFree(pRespTag->ResCurves);

	return;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Frees the Public Tag Structures
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 24, 1999
 *------------------------------------------------------------------*/
void SpDevSetFree (	SpDevStruct_t	*pDevSet)
{
	int	i, j, k;
	SpPlatStruct_t	*Platform;
	SpCombStruct_t	*Comb;
	SpSetStruct_t	*SetArray;

	Platform = pDevSet->PlatformArray;
	for (i = 0; i < pDevSet->NumOfPlatforms; i++, Platform++)
	{
		Comb = Platform->CombArray;
		for (j = 0; j < Platform->CombCount; j++, Comb++)
		{
			SetArray = Comb->SetArray;
			for (k = 0; k < Comb->SetCount; k++, SetArray++)
				SpFree(SetArray->Setting);
			SpFree(Comb->SetArray);
		}
		SpFree(Platform->CombArray);
	}
	SpFree(pDevSet->PlatformArray);
	return;
}

