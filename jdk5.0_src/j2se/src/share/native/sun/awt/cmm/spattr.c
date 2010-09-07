/*
 * @(#)spattr.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains functions for tag management.

				Created by lsh, September 15, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spattr.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spattr.c $
		$Revision: 18 $
		$Date: 3/12/03 3:06p $
		$Author: Stone $

	SCCS Revision:
		@(#)spattr.c	1.59	4/16/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993 - 2002               ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include <string.h>
#include <stdio.h>


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free memory allocated to TagValue by SpTagGetXXX.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 4, 1993
 *------------------------------------------------------------------*/
static void SpTagFreeTextDesc (
				SpTextDesc_t	FAR *TextDesc)
{
	SpFree (TextDesc->IsoStr);
	SpFree (TextDesc->UniStr);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free memory allocated to TagValue by SpTagGetXXX.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 4, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagFree (
				SpTagValue_t	FAR *Value)
{
	SpTagType_t		TagType;
	KpUInt32_t		Index;
	SpProfileSeqDescRecord_t	FAR *ProfSeqRecord;
	SpProfileSeqDescRecord2_t	FAR *ProfSeqRecord2;

	SpTagGetIdType (Value->TagId, &TagType);

	/* if this is an unknown tag id, set the tag type from the
	   tag type signature */	
	if (TagType == Sp_AT_Unknown)
		TagType = Value->TagType;

	switch (TagType) {
	case Sp_AT_Enum:
		break;

	case Sp_AT_Chromaticity:
		SpFree(Value->Data.Chromaticity.Coords);
		break;

	case Sp_AT_ColorantTable:
		SpFree(Value->Data.ColorTable.Colorant);
		break;

	case Sp_AT_ColorantOrder:
		break;

	case Sp_AT_CrdInfo:
		if (Value->Data.CrdData.ProdDesc.count > 0)
			SpFree (Value->Data.CrdData.ProdDesc.CRD_String);
		for (Index = 0; Index < 4; Index++)
		{
			if (Value->Data.CrdData.CrdInfo[Index].count > 0)
				SpFree (Value->Data.CrdData.CrdInfo[Index].CRD_String);
		}
		break;

	case Sp_AT_Curve:
		SpFree (Value->Data.Curve.Data);
		break;

	case Sp_AT_Data:
		SpFree (Value->Data.Data.Bytes);
		break;

	case Sp_AT_DateTime:
		break;

	case Sp_AT_DevSettings:
		SpDevSetFree(&Value->Data.DevSet);
		break;

	case Sp_AT_Lut:
		return SpLutFree (&Value->Data.Lut);

	case Sp_AT_Measurement:
		break;

	case Sp_AT_MultiLanguage:
		if (Value->TagType == Sp_AT_Text)
			SpFree (Value->Data.Text);
		else if (Value->TagType == Sp_AT_TextDesc)
			SpTagFreeTextDesc (&Value->Data.TextDesc);
		else
			SpFreeMultiLang(&Value->Data.MultiLang);
		break;

	case Sp_AT_NamedColors:
		SpFree (Value->Data.NamedColors.Colors);
		break;
	case Sp_AT_NamedColors2:
		SpFree (Value->Data.NamedColors2.Colors);
		break;

	case Sp_AT_ParametricCurve:
		SpFree (Value->Data.ParaCurve.ParaData.Parameters);
		break;

	case Sp_AT_ProfileSeqDesc:
		for (Index = 0, 
			ProfSeqRecord = Value->Data.ProfileSeqDesc.Records,
			ProfSeqRecord2 = Value->Data.ProfileSeqDesc2.Records;
			Index < Value->Data.ProfileSeqDesc2.Count; Index++) {

			if ((SpTypeMultiLanguage == ProfSeqRecord2->DeviceManufacturerDesc.TagType) ||
				(SpTypeMultiLanguage == ProfSeqRecord2->DeviceModelDesc.TagType)) {
				SpFreeDeviceDesc (&ProfSeqRecord2->DeviceManufacturerDesc);
				SpFreeDeviceDesc (&ProfSeqRecord2->DeviceModelDesc);
				ProfSeqRecord2++;
			} else {
				SpFreeDeviceDesc ((SpDeviceDesc_t *)&ProfSeqRecord->DeviceManufacturerDesc);
				SpFreeDeviceDesc ((SpDeviceDesc_t *)&ProfSeqRecord->DeviceModelDesc);
				ProfSeqRecord++;
			}
		}
		SpFree (Value->Data.ProfileSeqDesc.Records);
		break;

	case Sp_AT_ResponseCurve:
		SpRespFree(&Value->Data.Response);
		break;

	case Sp_AT_SF15d16:
		SpFree (Value->Data.SF15d16s.Values);
		break;

	case Sp_AT_Screening:
		SpFree (Value->Data.Screening.Screens);
		break;

	case Sp_AT_Signature:
		break;

	case Sp_AT_Text:
		SpFree (Value->Data.Text);
		break;

	case Sp_AT_TextDesc:
		SpTagFreeTextDesc (&Value->Data.TextDesc);
		break;

	case Sp_AT_Ucrbg:
		SpFree (Value->Data.Ucrbg.Ucr);
		SpFree (Value->Data.Ucrbg.bg);
		SpFree (Value->Data.Ucrbg.Desc);
		break;

	case Sp_AT_UF16d16:
		SpFree (Value->Data.UF16d16s.Values);
		break;

	case Sp_AT_UInt16:
		SpFree (Value->Data.UInt16s.Values);
		break;

	case Sp_AT_UInt32:
		SpFree (Value->Data.UInt32s.Values);
		break;

	case Sp_AT_UInt64:
		SpFree (Value->Data.UInt64s.Values);
		break;

	case Sp_AT_UInt8:
		SpFree (Value->Data.UInt8s.Values);
		break;

	case Sp_AT_Viewing:
		break;

	case Sp_AT_XYZ:
		break;

	case Sp_AT_Unknown:
		SpFree (Value->Data.Binary.Values);
		break;

	default:
		return SpStatBadTagType;
	}

	return SpStatSuccess;
}

/*--------------------------*/
/* define tag ID type table */
/*--------------------------*/

#define SPTAGID(nm, t) {SpTag##nm, Sp_AT_##t}

typedef struct {
	SpTagId_t		TagId;
	SpTagType_t		TagType;
} SpTagIdTbl_t;


/* TagIdTbl1 is used for Pre-version 4 profiles.
 TagIdTbl2 is used for version 4 (and later) profiles */
static SpTagIdTbl_t TagIdTbl1 [] = {

	SPTAGID (Unknown,		Unknown),

/* Kodak private tags */
	SPTAGID (KDeviceBits,		UInt32),
	SPTAGID (KCompressedLUT,	Unknown),
	SPTAGID (KDeviceSerialNum,	TextDesc),
	SPTAGID (KDeviceSettings,	TextDesc),
	SPTAGID (KDeviceUnit,		TextDesc),
	SPTAGID (KDMax,			SF15d16),
	SPTAGID (KEffectType,		Enum),
	SPTAGID (KIllum,		Enum),
	SPTAGID (KInterpretation,	Enum),
	SPTAGID (KLinearizationType,	TextDesc),
	SPTAGID (KLinearized,		Enum),
	SPTAGID (KMedium,		Enum),
	SPTAGID (KMediumDesc,		TextDesc),
	SPTAGID (KMediumProduct,	Enum),
	SPTAGID (KMediumSense,		Enum),
	SPTAGID (KDotGain25,		SF15d16),
	SPTAGID (KDotGain50,		SF15d16),
	SPTAGID (KDotGain75,		SF15d16),
	SPTAGID (KPhosphor,		Enum),
	SPTAGID (KPrtBlackShape,	Enum),
	SPTAGID (KPrtBlackStartDelay,	Enum),
	SPTAGID (KSenseInvertible,	Enum),
	SPTAGID (KVersion,		Text),
	SPTAGID (KDensityType,		TextDesc),
	SPTAGID (KProfileHistory,	Text),
	SPTAGID (KPCDFilmTerm,		Text),
	SPTAGID (KToneInfoDesc,		UInt8),
	SPTAGID (KCreationSoftware,	Text),

/* Kodak private tags for device to device profiles */
	SPTAGID (KXchDeviceBits,	UInt32),
	SPTAGID (KXchDeviceSerialNum,	TextDesc),
	SPTAGID (KXchDeviceSettings,	TextDesc),
	SPTAGID (KXchDeviceUnit,	TextDesc),
	SPTAGID (KXchGamma,		SF15d16),
	SPTAGID (KXchIllum,		Enum),
	SPTAGID (KXchInterpretation,	Enum),
	SPTAGID (KXchLinearizationType,	TextDesc),
	SPTAGID (KXchLinearized,	Enum),
	SPTAGID (KXchMedium,		Enum),
	SPTAGID (KXchMediumDesc,	TextDesc),
	SPTAGID (KXchMediumProduct,	Enum),
	SPTAGID (KXchMediumSense,	Enum),
	SPTAGID (KXchPhosphor,		Enum),
	SPTAGID (KXchSenseInvertible,	Enum),
	SPTAGID (KXchDotGain25,		SF15d16),
	SPTAGID (KXchDotGain50,		SF15d16),
	SPTAGID (KXchDotGain75,		SF15d16),

/* Kodak private tags for chaining rules */
	SPTAGID (KChainAToB0,		UInt8),
	SPTAGID (KChainBToA0,		UInt8),
	SPTAGID (KChainPreview0,	UInt8),

	SPTAGID (KChainAToB1,		UInt8),
	SPTAGID (KChainBToA1,		UInt8),
	SPTAGID (KChainPreview1,	UInt8),

	SPTAGID (KChainAToB2,		UInt8),
	SPTAGID (KChainBToA2,		UInt8),
	SPTAGID (KChainPreview2,	UInt8),

	SPTAGID (KChainGamut,		UInt8),

/* Kodak private tags for multi colorant xforms */
	SPTAGID (KInkName0,		Text),
	SPTAGID (KInkName1,		Text),
	SPTAGID (KInkName2,		Text),
	SPTAGID (KInkName3,		Text),
	SPTAGID (KInkName4,		Text),
	SPTAGID (KInkName5,		Text),
	SPTAGID (KInkName6,		Text),
	SPTAGID (KInkName7,		Text),
	SPTAGID (KInkDensities,		SF15d16),

/* Public tags */
	SPTAGID (AToB0,			Lut),
	SPTAGID (AToB1,			Lut),
	SPTAGID (AToB2,			Lut),
	SPTAGID (BlueColorant,		XYZ),
	SPTAGID (BlueTRC,		Curve),
	SPTAGID (BToA0,			Lut),
	SPTAGID (BToA1,			Lut),
	SPTAGID (BToA2,			Lut),
	SPTAGID (CalibrationDateTime,	DateTime),
	SPTAGID (CharTarget,		Text),
	SPTAGID (ChromaticAdapt,	SF15d16),
	SPTAGID (Chromaticity,		Chromaticity),
	SPTAGID (ColorantOrder,		ColorantOrder),
	SPTAGID (ColorantTable,		ColorantTable),
	SPTAGID (CopyRight,		Text),
	SPTAGID (DeviceMfgDesc,		TextDesc),
	SPTAGID (DeviceModelDesc,	TextDesc),
	SPTAGID (DevSettings,		DevSettings),
	SPTAGID (Gamut,			Lut),
	SPTAGID (GrayTRC,		Curve),
	SPTAGID (GreenColorant,		XYZ),
	SPTAGID (GreenTRC,		Curve),
	SPTAGID (Luminance,		XYZ),
	SPTAGID (Measurement,		Measurement),
	SPTAGID (MediaBlackPnt,		XYZ),
	SPTAGID (MediaWhitePnt,		XYZ),
	SPTAGID (NamedColor,		NamedColors),
	SPTAGID (Preview0,		Lut),
	SPTAGID (Preview1,		Lut),
	SPTAGID (Preview2,		Lut),
	SPTAGID (ProfileDesc,		TextDesc),
	SPTAGID (ProfileSeqDesc,	ProfileSeqDesc),
	SPTAGID (PS2CRD0,		Data),
	SPTAGID (PS2CRD1,		Data),
	SPTAGID (PS2CRD2,		Data),
	SPTAGID (PS2CRD3,		Data),
	SPTAGID (PS2CSA,		Data),
	SPTAGID (PS2RenderIntent,	Data),
	SPTAGID (RedColorant,		XYZ),
	SPTAGID (RedTRC,		Curve),
	SPTAGID (ResponseCurve,		ResponseCurve),
	SPTAGID (ScreeningDesc,		TextDesc),
	SPTAGID (Screening,		Screening),
	SPTAGID (Technology,		Signature),
	SPTAGID (Ucrbg,			Ucrbg),
	SPTAGID (ViewingCondDesc,	TextDesc),
	SPTAGID (ViewingConditions,	Viewing),
	SPTAGID (NamedColor2,		NamedColors2),
	SPTAGID (CrdInfo,		CrdInfo),

	SPTAGID (ENDLIST,				Unknown),
};


/* Version 4 and later table */
static SpTagIdTbl_t TagIdTbl2 [] = {

	SPTAGID (Unknown,		Unknown),

/* Kodak private tags */
	SPTAGID (KDeviceBits,		UInt32),
	SPTAGID (KCompressedLUT,	Unknown),
	SPTAGID (KDeviceSerialNum,	MultiLanguage),
	SPTAGID (KDeviceSettings,	MultiLanguage),
	SPTAGID (KDeviceUnit,		MultiLanguage),
	SPTAGID (KDMax,			SF15d16),
	SPTAGID (KEffectType,		Enum),
	SPTAGID (KIllum,		Enum),
	SPTAGID (KInterpretation,	Enum),
	SPTAGID (KLinearizationType,	MultiLanguage),
	SPTAGID (KLinearized,		Enum),
	SPTAGID (KMedium,		Enum),
	SPTAGID (KMediumDesc,		MultiLanguage),
	SPTAGID (KMediumProduct,	Enum),
	SPTAGID (KMediumSense,		Enum),
	SPTAGID (KDotGain25,		SF15d16),
	SPTAGID (KDotGain50,		SF15d16),
	SPTAGID (KDotGain75,		SF15d16),
	SPTAGID (KPhosphor,		Enum),
	SPTAGID (KPrtBlackShape,	Enum),
	SPTAGID (KPrtBlackStartDelay,	Enum),
	SPTAGID (KSenseInvertible,	Enum),
	SPTAGID (KVersion,		Text),
	SPTAGID (KDensityType,		MultiLanguage),
	SPTAGID (KProfileHistory,	Text),
	SPTAGID (KPCDFilmTerm,		Text),
	SPTAGID (KToneInfoDesc,		UInt8),
	SPTAGID (KCreationSoftware,	Text),

/* Kodak private tags for device to device profiles */
	SPTAGID (KXchDeviceBits,	UInt32),
	SPTAGID (KXchDeviceSerialNum,	MultiLanguage),
	SPTAGID (KXchDeviceSettings,	MultiLanguage),
	SPTAGID (KXchDeviceUnit,	MultiLanguage),
	SPTAGID (KXchGamma,		SF15d16),
	SPTAGID (KXchIllum,		Enum),
	SPTAGID (KXchInterpretation,	Enum),
	SPTAGID (KXchLinearizationType,	MultiLanguage),
	SPTAGID (KXchLinearized,	Enum),
	SPTAGID (KXchMedium,		Enum),
	SPTAGID (KXchMediumDesc,	MultiLanguage),
	SPTAGID (KXchMediumProduct,	Enum),
	SPTAGID (KXchMediumSense,	Enum),
	SPTAGID (KXchPhosphor,		Enum),
	SPTAGID (KXchSenseInvertible,	Enum),
	SPTAGID (KXchDotGain25,		SF15d16),
	SPTAGID (KXchDotGain50,		SF15d16),
	SPTAGID (KXchDotGain75,		SF15d16),

/* Kodak private tags for chaining rules */
	SPTAGID (KChainAToB0,		UInt8),
	SPTAGID (KChainBToA0,		UInt8),
	SPTAGID (KChainPreview0,	UInt8),

	SPTAGID (KChainAToB1,		UInt8),
	SPTAGID (KChainBToA1,		UInt8),
	SPTAGID (KChainPreview1,	UInt8),

	SPTAGID (KChainAToB2,		UInt8),
	SPTAGID (KChainBToA2,		UInt8),
	SPTAGID (KChainPreview2,	UInt8),

	SPTAGID (KChainGamut,		UInt8),

/* Kodak private tags for multi colorant xforms */
	SPTAGID (KInkName0,		Text),
	SPTAGID (KInkName1,		Text),
	SPTAGID (KInkName2,		Text),
	SPTAGID (KInkName3,		Text),
	SPTAGID (KInkName4,		Text),
	SPTAGID (KInkName5,		Text),
	SPTAGID (KInkName6,		Text),
	SPTAGID (KInkName7,		Text),
	SPTAGID (KInkDensities,		SF15d16),

/* Public tags */
	SPTAGID (AToB0,			Lut),
	SPTAGID (AToB1,			Lut),
	SPTAGID (AToB2,			Lut),
	SPTAGID (BlueColorant,		XYZ),
	SPTAGID (BlueTRC,		Unknown),
	SPTAGID (BToA0,			Lut),
	SPTAGID (BToA1,			Lut),
	SPTAGID (BToA2,			Lut),
	SPTAGID (CalibrationDateTime,	DateTime),
	SPTAGID (CharTarget,		Text),
	SPTAGID (ChromaticAdapt,	SF15d16),
	SPTAGID (Chromaticity,		Chromaticity),
	SPTAGID (CopyRight,		MultiLanguage),
	SPTAGID (DeviceMfgDesc,		MultiLanguage),
	SPTAGID (DeviceModelDesc,	MultiLanguage),
	SPTAGID (DevSettings,		DevSettings),
	SPTAGID (Gamut,			Lut),
	SPTAGID (GrayTRC,		Unknown),
	SPTAGID (GreenColorant,		XYZ),
	SPTAGID (GreenTRC,		Unknown),
	SPTAGID (Luminance,		XYZ),
	SPTAGID (Measurement,		Measurement),
	SPTAGID (MediaBlackPnt,		XYZ),
	SPTAGID (MediaWhitePnt,		XYZ),
	SPTAGID (Preview0,		Lut),
	SPTAGID (Preview1,		Lut),
	SPTAGID (Preview2,		Lut),
	SPTAGID (ProfileDesc,		MultiLanguage),
	SPTAGID (ProfileSeqDesc,	ProfileSeqDesc),
	SPTAGID (PS2CRD0,		Data),
	SPTAGID (PS2CRD1,		Data),
	SPTAGID (PS2CRD2,		Data),
	SPTAGID (PS2CRD3,		Data),
	SPTAGID (PS2CSA,		Data),
	SPTAGID (PS2RenderIntent,	Data),
	SPTAGID (RedColorant,		XYZ),
	SPTAGID (RedTRC,		Unknown),
	SPTAGID (ResponseCurve,		ResponseCurve),
	SPTAGID (ScreeningDesc,		MultiLanguage),
	SPTAGID (Screening,		Screening),
	SPTAGID (Technology,		Signature),
	SPTAGID (Ucrbg,			Ucrbg),
	SPTAGID (ViewingCondDesc,	MultiLanguage),
	SPTAGID (ViewingConditions,	Viewing),
	SPTAGID (NamedColor2,		NamedColors2),
	SPTAGID (CrdInfo,		CrdInfo),
	SPTAGID (ColorantTable,		ColorantTable),
	SPTAGID (ColorantOrder,		ColorantOrder),

	SPTAGID (ENDLIST,				Unknown),
};


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert an tag ID to a type.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 29, 1993
 *------------------------------------------------------------------*/
void KSPAPI SpTagGetType (KpUInt32_t	ProfVers,
				SpTagId_t	TagId,
				SpTagType_t	FAR *TagType)
{
	SpTagIdTbl_t	*Entry;
	SpTagIdTbl_t	*RetEntry = TagIdTbl2;

	/* Use Old Tag Types */
	if (ProfVers < SpProfileTagVersion)
		RetEntry = TagIdTbl1;

/* look for ID */
	for (Entry = RetEntry; SpTagENDLIST != Entry->TagId; Entry++) {
		if (TagId == Entry->TagId)
		{
			RetEntry = Entry;
			break;
		}
	}

/* give caller the type */
/* Note:  If the type was not found, the first entry in the     */
/* table, Sp_AT_Unknown,  is returned                           */
/*    This is the correct action since we want to pass the      */
/* information along without any interpretations                */
	*TagType = RetEntry->TagType;
}

void KSPAPI SpTagGetIdType (
				SpTagId_t	TagId,
				SpTagType_t	FAR *TagType)
{
	SpTagGetType(SpProfileVersion, TagId, TagType);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Check for header needed to interperate data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 15, 1994
 *------------------------------------------------------------------*/
static KpBool_t SpTagNeedHeader (
				SpTagId_t	TagId)
{
	KpBool_t BoolRes = KPFALSE;

	if (TagId == SpTagNamedColor)
		BoolRes = KPTRUE;

	if (TagId == SpTagProfileSeqDesc)
		BoolRes = KPTRUE;

#if defined (PSFUNC)
	if (TagId == SpTagPS2CSA)
	   BoolRes = KPTRUE;
#endif

	return (BoolRes);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get public tag given its ID.
 *	If the tag is MultiLang return Ascii
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagGetById (
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				SpTagValue_t	FAR *Value)
{
	SpStatus_t	Status;
	SpTagType_t		TagType;
	SpUnicodeInfo_t	StringInfo;
	KpInt32_t		StringLength;
	KpChar_p		Text;
	KpInt16_t		Language = 0;
	KpInt16_t		Country = 0;

	Status = SpTagGetByIdEx (Profile, TagId, Value);
	if (SpStatSuccess == Status)
	{
		if (Sp_AT_MultiLanguage == Value->TagType)
		{
			StringInfo = Value->Data.MultiLang.StringInfo[0];
			StringLength = StringInfo.StringLength + 1;
			Text = SpMalloc (StringLength);

			SpTagGetType(SPICCVER23, TagId, &TagType);
			if (Sp_AT_Text == TagType)
			{
				Status = MultiLangToMLString(Value, &Language, &Country, 
												&StringLength, Text);
				SpFreeMultiLang(&Value->Data.MultiLang);
				Value->TagType = TagType;
				Value->Data.Text = Text;
			}
			else if (Sp_AT_TextDesc == TagType)
			{
				Status = MultiLangToMLString(Value, &Language, &Country, 
												&StringLength, Text);
				SpFreeMultiLang(&Value->Data.MultiLang);
				Status = SpStringToTextDesc(Text, &Value->Data.TextDesc);
				Value->TagType = TagType;
				SpFree (Text);
			} else {
				SpFree (Text);	/* what's going on? something must be wrong! */
			}
		}
	}
	return (Status);
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get public tag given its ID.
 *
 * AUTHOR
 * 	msm
 *
 * DATE CREATED
 *	March 12, 2002
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagGetByIdEx (
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				SpTagValue_t	FAR *Value)
{
	SpStatus_t	Status;
	KpUInt32_t	TagDataSize;
	void		KPHUGE * TagData;
	SpHeader_t	Hdr;
	SpHeader_t	FAR *Header;
	void		KPHUGE *	tagDataH;
#if defined (PSFUNC)
	SpTransRender_t	render;
#endif

/* get the profile header, if needed */
	if (SpTagNeedHeader (TagId)) {
		Status = SpProfileGetHeader (Profile, &Hdr);
		if (SpStatSuccess != Status)
			return Status;

		Header = &Hdr;
	}
	else
		Header = NULL;

/* get the tag data */
	Status = SpRawTagDataGet (Profile, TagId, &TagDataSize, &tagDataH);
#if defined (PSFUNC)
	if (SpStatSuccess != Status)
	{
		if (TagId == SpTagPS2CRD0)
		{
			Status = SpPSCRDCreate(Profile, 
					SpTransRenderPerceptual,
					&tagDataH, &TagDataSize);

		} else if (TagId == SpTagPS2CRD1)
		{
			Status = SpPSCRDCreate(Profile, 
					SpTransRenderColormetric,
					&tagDataH, &TagDataSize);

		} else if (TagId == SpTagPS2CRD2)
		{
			Status = SpPSCRDCreate(Profile, 
					SpTransRenderSaturation,
					&tagDataH, &TagDataSize);

		} else if (TagId == SpTagPS2CRD3)
		{
			Status = SpStatNotImp;

		} else if (TagId == SpTagPS2CSA)
		{
			/* Definitions of header rendering intent are different
			   from those for specifying xform rendering intent
 			   by 1.  See sprofile.h */
			Status = SpPSCSACreate(Profile, 
					Hdr.RenderingIntent +1,
					&tagDataH, &TagDataSize);
		}
	}
#endif

	if (SpStatSuccess != Status)
		return Status;

/* convert the tag data to a public form */
	TagData = (void KPHUGE FAR *) lockBuffer (tagDataH) ;
	Status = SpTagToPublic (Header, TagId, TagDataSize, TagData, Value);

/* "free" local copy */
	SpRawTagDataFree (Profile, TagId, TagData);
    unlockBuffer (tagDataH);

	return Status;
}


#if !defined (SP_NO_TAGSET)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Set tag value.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagSet (
				SpProfile_t		Profile,
				SpTagValue_t	FAR *Value)
{
	SpStatus_t		Status;
	SpHeader_t		Hdr;
	SpHeader_t		FAR *Header;
	SpTagType_t		TagType;
	void			KPHUGE *Buf;
	KpUInt32_t		BufSize;

	Status = SpProfileGetHeader (Profile, &Hdr);
	if (SpStatSuccess != Status)
		return Status;

/* Validate type for 'known' tags */
	SpTagGetType (Hdr.ProfileVersion, Value->TagId, &TagType);
	if ((TagType != Sp_AT_Unknown) && (TagType != Value->TagType))
		return SpStatBadTagType;

/* get the profile header, if needed */
	if (SpTagNeedHeader (Value->TagId))
		Header = &Hdr;
	else
		Header = NULL;

/* make new tag value reference */
	Status = SpTagFromPublic (Header, Value, &BufSize, &Buf);
	if (SpStatSuccess != Status)
		return Status;

/* now save the data to the profile after checking if Lut Tag and Type match */
	Status = SpTagTestLut(Value->TagId, Buf);
	if (Status == SpStatSuccess)
		Status = SpRawTagDataSet (Profile, Value->TagId, BufSize, Buf);

/* free the buffer containing the InterColor formatted data */
	SpFree (Buf);

	return Status;
}
#endif


     
     
