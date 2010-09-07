/*
 * @(#)sprofprv.h	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This header defines the Private interface to the Profile
				Processor.

				Created by lsh, October 25, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sprofprv.h $
		$Logfile: /DLL/KodakCMS/sprof_lib/h/sprofprv.h $
		$Revision: 9 $
		$Date: 9/25/02 9:52a $
		$Author: Arourke $

	SCCS Revision:
	 @(#)sprofprv.h	1.72 3/2/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-2002                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/
#ifndef SPROFPRIV_H
#define SPROFPRIV_H

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#if defined (KPWIN32)
#pragma pack (push, SpvLevel, 4)
#endif

#include "kcms_sys.h"
#include "kcmptdef.h"

/* Composition types */
typedef KpInt32_t SpConnectType_t;

#define SpConnect_Type_Mask		0x0f
#define	SpConnect_Std		0x0
#define	SpConnect_pf_8		0x1
#define	SpConnect_pf_16		0x2
#define	SpConnect_pf		0x3
#define	SpConnect_Serial	0x4

#define SpConnect_Method_Mask	0xf0
#define SpConnect_Default		0x00
#define SpConnect_Chain			0x10
#define SpConnect_Chain_Std		SpConnect_Chain | SpConnect_Std
#define SpConnect_Chain_pf_8	SpConnect_Chain | SpConnect_pf_8
#define SpConnect_Chain_pf_16	SpConnect_Chain | SpConnect_pf_16
#define SpConnect_Chain_pf		SpConnect_Chain | SpConnect_pf

#define SpConnect_Combine		0x20
#define SpConnect_Combine_Std	SpConnect_Combine | SpConnect_Std
#define SpConnect_Combine_pf_8	SpConnect_Combine | SpConnect_pf_8
#define SpConnect_Combine_pf_16	SpConnect_Combine | SpConnect_pf_16
#define SpConnect_Combine_pf	SpConnect_Combine | SpConnect_pf

#define SpConnect_Mode_Mask		0xf00
#define	SpConnect_Largest		0x100

/*	Private color spaces to be used within the profile processor.  These	
	color spaces suppliment the color spaces defined in sprofile.h			*/
#define SpSpaceRCS 					SpSigMake('R', 'C', 'S', ' ')

/* Add Signatures for Originator that flag Data Type of KCM_ICC_TYPE_0 */
#define SpSigOrgKodak1_Ver_0 SpSigMake('K', 'O', 'D', 'A')
#define SpSigOrgKodak2_Ver_0 SpSigMake('K', 'O', 'D', 'K')

/* Add Signatures for Originator that flag Data Type of KCM_ICC_TYPE_1 */
#define SpSigOrgKodak_Type_1 SpSigMake('K', 'O', 'D', '1')

/* Valid Paradigm Types for lab<->uvl conversions */
#define	SpParadigmRel	0	/* relative paradigm type */
#define	SpParadigmAbs	1	/* absolute paradigm type */

typedef KpInt32_t SpParadigm_t;
typedef PTRelToAbs_t SpRelToAbs_t;

/* F15d16 values for D50 */
#define SP_D50_F15D16_X	0xF6D6	/* 0.9642	*/
#define SP_D50_F15D16_Y	0x10000	/* 1.0		*/
#define SP_D50_F15D16_Z	0xD32D	/* 0.8249	*/

/* Defined for backward compatibility */
#define SpXformFromMFut	SpXformFromBuffer

#if defined (KPMAC)
void SpGetCPInstance(
				KpInt32_t *theCurCP);
				
void SpSetCPInstance(
				KpInt32_t theCurCP);
#endif

SpStatus_t KSPAPI SpXformFromPTRefNumEx (
				SpConnectType_t	ConnectType,
				SpParadigm_t	ParadigmType,
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform);

/*	For importing Raw PTs, no RCS to LAB conversion	*/
SpStatus_t KSPAPI SpXformFromPTRefNumNC (
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform);


/* Do Combine of PTs only - no chaining */
SpStatus_t KSPAPI SpXformFromPTRefNumCombine (
				SpConnectType_t	ConnectType,
				SpParadigm_t	ParadigmType,
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformFromPTRefNum (
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformGetRefNum (
				SpXform_t		Xform,
				PTRefNum_t		FAR *RefNum);

SpStatus_t KSPAPI SpXformCreateFromDataEx (
				SpConnectType_t	ConnectType,
				KpInt32_t		Size,
				KpLargeBuffer_t	Data,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformCreateMatTagsFromPT (
				SpProfile_t		Profile,
				PTRefNum_t		RefNum);

SpStatus_t KSPAPI SpXformCreateMatTagsFromXform (
				SpProfile_t		Profile,
				SpXform_t		Xform);

SpStatus_t KSPAPI SpXformCreateMatTags (
				SpProfile_t		Profile,
				KpInt32_t		DataSize,
				KpLargeBuffer_t	Data);

SpStatus_t KSPAPI SpXformLCSCreate (
				KpF15d16XYZ_t	FAR *rXYZ,
				KpF15d16XYZ_t	FAR *gXYZ,
				KpF15d16XYZ_t	FAR *bXYZ,
				KpResponse_t	FAR *rTRC,
				KpResponse_t	FAR *gTRC,
				KpResponse_t	FAR *bTRC,
				KpUInt32_t	gridsize, 
				KpBool_t	invert, 
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformLCSAdaptCreate (
				KpF15d16XYZ_t	FAR *rXYZ,
				KpF15d16XYZ_t	FAR *gXYZ,
				KpF15d16XYZ_t	FAR *bXYZ,
				KpResponse_t	FAR *rTRC,
				KpResponse_t	FAR *gTRC,
				KpResponse_t	FAR *bTRC,
				KpUInt32_t		gridsize, 
				KpBool_t		invert, 
				KpBool_t		adapt, 
				KpBool_t		lagrange, 
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformCreate (
				KpF15d16XYZ_t	FAR *rXYZ,
				KpF15d16XYZ_t	FAR *gXYZ,
				KpF15d16XYZ_t	FAR *bXYZ,
				KpResponse_t	FAR *rTRC,
				KpResponse_t	FAR *gTRC,
				KpResponse_t	FAR *bTRC,
				KpUInt32_t	gridsize,
				KpBool_t	invert,
				KpBool_t	adapt,
				KpBool_t	lagrange,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformGrayCreate (
				KpResponse_t	FAR *gTRC,
				KpUInt32_t	gridsize,
				KpBool_t	invert,
				SpXform_t	FAR *Xform);
 
SpStatus_t KSPAPI SpXformGetParms (
				SpXform_t	Xform,
				SpTransRender_t	*WhichRender,
				SpTransType_t	*WhichTransform,
				KpF15d16XYZ_t	*HdrWhite,
				KpF15d16XYZ_t	*MedWhite,
				KpUInt32_t	*ChainIn,
				KpUInt32_t	*ChainOut);

SpStatus_t KSPAPI SpXformSetParms (
				SpXform_t	Xform,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				KpF15d16XYZ_t	HdrWhite,
				KpF15d16XYZ_t	MedWhite,
				KpUInt32_t	ChainIn,
				KpUInt32_t	ChainOut);

SpStatus_t KSPAPI SpXformToBlobGetDataSize (
				SpXform_t		Xform,
				KpInt32_t		FAR *BufferSize);

SpStatus_t KSPAPI SpXformToBlobGetData (
				SpXform_t		Xform,
				KpInt32_t		BufferSize,
				SpHugeBuffer_t	Buffer);

SpStatus_t KSPAPI SpXformFromBlob (
				KpInt32_t		BufferSize,
				SpHugeBuffer_t	Buffer,
				SpXform_t		FAR *Xform);

SpStatus_t KSPAPI SpXformGenerateDisplay (
				SpProfile_t	Profile,
				KpUInt32_t	GridSize,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTrans,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpConnectSequenceEx (
				SpConnectType_t	ConnectType,
				KpInt32_t		XformCnt,
				SpXform_t		FAR *XformsSequence,
				SpXform_t		FAR *Result,
				KpInt32_t		FAR *FailingXform,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpConnectSequence (
				KpInt32_t		XformCnt,
				SpXform_t		FAR *XformsSequence,
				SpXform_t		FAR *Result,
				KpInt32_t		FAR *FailingXform,
				SpProgress_t	ProgressFunc,
				void			FAR *Data);

SpStatus_t KSPAPI SpXformWtPtGet(SpRelToAbs_t	*Rel2AbsStruct,
				KpInt32_t	mode,
				SpXform_t	*SpXform);

/*******************************************************************/
/* Raw Data Access (See Documentation before using these Function) */
/*******************************************************************/

SpStatus_t KSPAPI SpRawTagDataGetSize (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	FAR *TagDataSize);

SpStatus_t KSPAPI SpRawTagDataGet (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	FAR *TagDataSize,
				void		KPHUGE * FAR *TagData);

void KSPAPI SpRawTagDataFree (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				void		KPHUGE *TagData);

SpStatus_t KSPAPI SpRawTagDataSet (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	TagDataSize,
				void		KPHUGE *TagData);

SpStatus_t KSPAPI SpRawHeaderGet (
				SpProfile_t	Profile,
				KpUInt32_t	BufferSize,
				void		KPHUGE *Buffer);

/********************************************************************/
/* Misc																*/
/********************************************************************/

#if !defined (KPSUN)
SpStatus_t KSPAPI SpCvrtIOFileData (
				SpIOFileChar_t	*SpProps,
				ioFileChar		*KcmProps);

SpStatus_t KSPAPI SpCvrtIOFileProps(
				SpIOFileChar_t	*SpProps,
				SpFileProps_t	*KcmProps);

SpStatus_t KSPAPI SpCvrtSpFileData (
				SpFileProps_t	*SpProps,
				ioFileChar  	*KcmProps);

SpStatus_t KSPAPI SpCvrtSpFileProps(
				SpFileProps_t	*SpProps,
				KpFileProps_t	*KcmProps);
#endif

SpStatus_t KSPAPI SpXformFromLutDT(
				KpInt32_t	DataType,
				SpLut_t		Lut,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				KpF15d16XYZ_t	HdrWhite,
				KpF15d16XYZ_t	MedWhite,
				KpUInt32_t	ChainIn,
				KpUInt32_t	ChainOut,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformFromLut(SpLut_t		Lut,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				KpF15d16XYZ_t	HdrWhite,
				KpF15d16XYZ_t	MedWhite,
				KpUInt32_t	ChainIn,
				KpUInt32_t	ChainOut,
				SpXform_t	FAR *Xform);

SpStatus_t KSPAPI SpXformToLut  (SpXform_t	Xform,
				SpLut_t		*Lut,
				SpTransRender_t	*WhichRender,
				SpTransType_t	*WhichTransform,
				SpSig_t		*SpaceIn,
				SpSig_t		*SpaceOut,
				KpF15d16XYZ_t	*HdrWhite,
				KpF15d16XYZ_t	*MedWhite,
				KpUInt32_t	*ChainIn,
				KpUInt32_t	*ChainOut);

SpStatus_t KSPAPI SpXformToLutDT(SpXform_t	Xform,
				KpInt32_t	DataType,
				SpLut_t		*Lut,
				SpTransRender_t	*WhichRender,
				SpTransType_t	*WhichTransform,
				SpSig_t		*SpaceIn,
				SpSig_t		*SpaceOut,
				KpF15d16XYZ_t	*HdrWhite,
				KpF15d16XYZ_t	*MedWhite,
				KpUInt32_t	*ChainIn,
				KpUInt32_t	*ChainOut);


SpStatus_t KSPAPI SpTagGetString(SpTagValue_t *TagValue,
				  KpInt32_p	BufSize,
			  	KpChar_p	Buffer);

SpStatus_t KSPAPI SpTagGetMLString(SpTagValue_t *TagValue,
				KpInt16_p	Language,
				KpInt16_p	Country,
				KpInt32_p	BufSize,
			  	KpChar_p	Buffer);

SpStatus_t KSPAPI SpProfileGetHeaderString(SpSearchType_t	hdrItem,
				SpHeader_t		*hdr,
				KpInt32_p		BufSize,
				KpChar_p		Buffer);

SpStatus_t SpStringToTextDesc(
			KpChar_p	pString,
			SpTextDesc_t	*pTextDesc);
 
SpStatus_t SpStringToMultiLang(
			KpChar_p	pString,
			KpInt16_t	Language,
			KpInt16_t	Country,
			SpMultiLang_t	*pMultiLang);
 
KpBool_t KSPAPI SpProfileValidHandle(SpProfile_t SpProf);

/************************************************/
/* Functions we want to phase out of public use */
/************************************************/
SpStatus_t KSPAPI SpProfileSaveToFileEx (
				SpProfile_t Profile,
				char		FAR *Name,
				KpBool_t		ShareTags);

SpStatus_t KSPAPI SpProfileSaveToFile (
				SpProfile_t Profile,
				char		FAR *Name);

SpStatus_t KSPAPI SpProfileLoadFromFile (
				SpCallerId_t	CallerId,
				char			FAR *FileName,
				SpProfile_t		FAR *Profile);

SpStatus_t KSPAPI SpProfileGetFileName (
				SpProfile_t		Profile,
				size_t			BufferSize,
				char			FAR *Buffer);

SpStatus_t KSPAPI SpProfileSetFileName (
				SpProfile_t		Profile,
				char			FAR *FileName);
SpStatus_t KSPAPI SpProfileGetSharedTags(
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				SpTagId_t		*Matched_TagIds,
				KpInt32_t		*num_matched_tags);

SpStatus_t KSPAPI SpProfileGetTagCount (
				SpProfile_t Profile,
				KpInt32_t		*tagCount);

#if defined (KPWIN32)
#pragma pack (pop, SpvLevel)
#endif


#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#endif	/* SPROFPRIV_H */
     
