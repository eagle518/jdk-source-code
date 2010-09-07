/*
 * @(#)spxf_gen.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the transform functions.

				Created by lsh, September 20, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1997 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spxf_gen.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spxf_gen.c $
		$Revision: 7 $
		$Date: 1/31/02 11:28a $
		$Author: Stone $

	SCCS Revision:
	@(#)spxf_gen.c	1.60	3/3/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-1997                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include "attrcipg.h"
#include <stdio.h>
#include <string.h>

#if defined (SP_WRITE_PTS)
static void getPTFileName(KpChar_p ptFileName);
static void savePT(KpChar_p ptFileName, KpUInt32_t size, SpHugeBuffer_t ptData);
#endif


/*--------------------------------------------------------------------*/
SpStatus_t SpSetKcmAttrInt (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	AttrValue)
{
	PTErr_t		PTErr;
	char		AttrStrValue [33];

	if ((AttrNum == KCM_IN_CHAIN_CLASS_2) || (AttrNum == KCM_OUT_CHAIN_CLASS_2)) {
		if (AttrValue == 0) {
			PTErr = PTSetAttribute (RefNum, AttrNum, NULL);
		} else {
			KpItoa (AttrValue, AttrStrValue);
			PTErr = PTSetAttribute (RefNum, AttrNum, AttrStrValue);
		}
	} else {
		KpItoa (AttrValue, AttrStrValue);
		PTErr = PTSetAttribute (RefNum, AttrNum, AttrStrValue);
	}
	return SpStatusFromPTErr(PTErr);
}

/*--------------------------------------------------------------------*/
KpInt32_t SpGetKcmAttrInt (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum)
{
	PTErr_t		PTErr;
	char		AttrValue [10];
	KpInt32_t	AttrSize;

	AttrSize = sizeof (AttrValue);
	PTErr = PTGetAttribute (RefNum, AttrNum, &AttrSize, AttrValue);
	if (KCP_SUCCESS != PTErr)
		return KCM_UNKNOWN;

	return KpAtoi (AttrValue);
}


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
SpStatus_t SpXformFromPTRefNumImp (
				PTRefNum_t	PTRefNum,
				SpXform_t	FAR *Xform)
{
	SpXformData_t	FAR *XformData;
	SpStatus_t		Status;
#if defined (SP_WRITE_PTS)
	KpUInt32_t		size;
	SpHugeBuffer_t	ptData;
	char			ptFileName[256];
#endif

	*Xform = NULL;

/* allocate a Transform block */
	Status = SpXformAllocate (Xform);
	if (SpStatSuccess != Status)
		return Status;

/* lock down the Xform */
	XformData = SpXformLock (*Xform);
	if (NULL == XformData)
	{
		SpXformFree (Xform);
		*Xform = NULL;
		return SpStatBadXform;
	}

/* set the xform data */
	XformData->PTRefNum = PTRefNum;

	Status = SpXformGetColorSpace (PTRefNum, KCM_SPACE_IN, &XformData->SpaceIn);
	if (SpStatSuccess == Status)
		Status = SpXformGetColorSpace (PTRefNum, KCM_SPACE_OUT, &XformData->SpaceOut);


	XformData->ChainIn = SpGetKcmAttrInt (PTRefNum, KCM_IN_CHAIN_CLASS_2);
	XformData->ChainOut = SpGetKcmAttrInt (PTRefNum, KCM_OUT_CHAIN_CLASS_2);

	XformData->LutType = SpTypeLut16;
	XformData->LutSize = SP_ICC_MFT2;
	XformData->WhichRender = SpTransRenderAny;
	XformData->WhichTransform = SpTransTypeUnknown;

	if (SpStatSuccess != Status) {
		SpXformFree (Xform);
		*Xform = NULL;
		return Status;
	}

	SpXformUnlock (*Xform);
	
#if defined (SP_WRITE_PTS)
/* check to see if this xform should be written to disk */
	if (writePTs())
	{
		/* get the PT data from the CP */
		Status = SpXformGetDataFromCP(PTRefNum, 0, NULL, 
					&size, &ptData);
		if (Status != SpStatSuccess) return SpStatSuccess;

		/* get the name of pt to be saved */
		getPTFileName(ptFileName);

		/* write the PT to disk */
		savePT(ptFileName, size, ptData);

		SpFree(ptData);
	}
#endif
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *      Create transform given colorspace definition for Grayscale
 *
 * AUTHOR
 *      dho
 *
 * DATE CREATED
 *      September 6, 1995
*------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformGrayCreate (
				KpResponse_t    FAR *gTRC,
				KpUInt32_t      gridsize,
				KpBool_t        invert,
				SpXform_t       FAR *Xform)
{
	PTRefNum_t              PTRefNum;
	PTErr_t                 PTStat;
	ResponseRecord_t	gRespRec;

	*Xform = NULL;

	SpCurveToResponseRec(gTRC, &gRespRec);

	PTStat = PTNewMonoPT(&gRespRec,
				gridsize, invert, &PTRefNum);
	if (PTStat != KCP_SUCCESS)
		return SpStatusFromPTErr(PTStat);

	/* build a profile transform structure */
	return SpXformFromPTRefNumImp (PTRefNum, Xform);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create transform given XYZ logical colorspace definition
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	January 31, 1996
 *------------------------------------------------------------------*/

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
				SpXform_t	FAR *Xform)
{
	PTRefNum_t		PTRefNum;
	PTErr_t			PTStat;
	SpStatus_t		Status;
	newMGmode_t		PTmodes;
	KpInt32_t		AttrValue;
	KpInt32_t		ChainValue;
	ResponseRecord_t	rRespRec, gRespRec, bRespRec;

	*Xform = NULL;

/* Make the fut - set the modes  */
	/* no adaptation for pt2pf display pf generation */
	if (adapt)
		PTmodes.adaptMode = KCP_BRADFORDD50_ADAPTATION; 
	else
		PTmodes.adaptMode = KCP_NO_ADAPTATION; 

	if (lagrange) 
		PTmodes.interpMode = KCP_TRC_LAGRANGE4_INTERP_LAB;
		
	else 
		PTmodes.interpMode = KCP_TRC_LINEAR_INTERP_LAB;

	/* LAB Color Space */
	AttrValue = KCM_CIE_LAB;
	ChainValue = KCM_CHAIN_CLASS_CIELAB1;

	SpCurveToResponseRec(rTRC, &rRespRec);
	SpCurveToResponseRec(gTRC, &gRespRec);
	SpCurveToResponseRec(bTRC, &bRespRec);
	PTStat = PTNewMatGamAIPT((FixedXYZColor FAR *) rXYZ,
				(FixedXYZColor FAR *) gXYZ,
				(FixedXYZColor FAR *) bXYZ,
				&rRespRec,
				&gRespRec,
				&bRespRec, 
				gridsize, invert, 
				&PTmodes, &PTRefNum);

	/* Failure of LAB requires trying the old way */
	if (KCP_SUCCESS != PTStat)
	{
		if (lagrange) 
			PTmodes.interpMode = KCP_TRC_LAGRANGE4_INTERP;
		else 
			PTmodes.interpMode = KCP_TRC_LINEAR_INTERP;

		/* Failed LAB, getting XYZ */
		AttrValue = KCM_CIE_XYZ;
		ChainValue = KCM_CHAIN_CLASS_XYZ;

		SpCurveToResponseRec(rTRC, &rRespRec);
		SpCurveToResponseRec(gTRC, &gRespRec);
		SpCurveToResponseRec(bTRC, &bRespRec);
		PTStat = PTNewMatGamAIPT((FixedXYZColor FAR *) rXYZ,
				(FixedXYZColor FAR *) gXYZ,
				(FixedXYZColor FAR *) bXYZ,
				&rRespRec,
				&gRespRec,
				&bRespRec, 
				gridsize, invert, 
				&PTmodes, &PTRefNum);
		if (KCP_SUCCESS != PTStat)
			return SpStatusFromPTErr(PTStat);
	}

/* set the color space */
	if (invert) {
		Status = SpSetKcmAttrInt (PTRefNum, KCM_SPACE_IN, AttrValue);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, KCM_RGB);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					ChainValue);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
	}
	else {
		Status = SpSetKcmAttrInt (PTRefNum, KCM_SPACE_IN, KCM_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_SPACE_OUT, AttrValue);

		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_IN_CHAIN_CLASS_2, 
					KCM_CHAIN_CLASS_MON_RGB);
		if (SpStatSuccess == Status)
			Status = SpSetKcmAttrInt (PTRefNum, 
					KCM_OUT_CHAIN_CLASS_2, 
					ChainValue);
	}
	if (SpStatSuccess != Status)
		return Status;

/* build a profile transform structure */
	return SpXformFromPTRefNumImp (PTRefNum, Xform);
}


SpStatus_t SetWtPt(SpProfile_t Profile,
			  SpXformData_t *XformData)
{
SpStatus_t	Status;
SpTagValue_t	TagValue;
SpHeader_t	Header;



	Status = SpProfileGetHeader(Profile, &Header);

	if (SpStatSuccess == Status)
	{
		XformData->HdrWtPoint.X = Header.Illuminant.X;
		XformData->HdrWtPoint.Y = Header.Illuminant.Y;
		XformData->HdrWtPoint.Z = Header.Illuminant.Z;
		XformData->HdrWPValid   = KPTRUE;
	} else {
		XformData->HdrWtPoint.X = 0;
		XformData->HdrWtPoint.Y = 0;
		XformData->HdrWtPoint.Z = 0;
		XformData->HdrWPValid   = KPFALSE;
	}

	Status = SpTagGetById (Profile, SpTagMediaWhitePnt, &TagValue);
 
	if (SpStatSuccess == Status)
	{
		XformData->MedWtPoint.X = TagValue.Data.XYZ.X;
		XformData->MedWtPoint.Y = TagValue.Data.XYZ.Y;
		XformData->MedWtPoint.Z = TagValue.Data.XYZ.Z;
		XformData->MedWPValid   = KPTRUE;

		SpTagFree (&TagValue);
	} else {
		XformData->MedWtPoint.X = 0;
		XformData->MedWtPoint.Y = 0;
		XformData->MedWtPoint.Z = 0;
		XformData->MedWPValid   = KPFALSE;
	}

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Generate the requested transform form profile tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 29, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformGenerate (
				SpProfile_t		Profile,
				KpUInt32_t		GridSize,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTrans,
				SpXform_t		FAR *Xform)
{
	SpStatus_t		Status;
	SpXform_t		Xforms [2];
	KpInt32_t		FailXform;
	KpBool_t		Invert;
	KpBool_t		Linear = KPFALSE, Adapt = KPFALSE;
	KpF15d16XYZ_t	rXYZ, gXYZ, bXYZ;
	SpTagValue_t	Value, rTRC, gTRC, bTRC;
	SpXformData_t	*XformData = NULL;
	SpTransRender_t FoundRender;

	SPArgUsed (WhichRender);

	*Xform = NULL;

/* Determine the direction of the transform */
	switch (WhichTrans) {
	case SpTransTypeIn:
		Invert = KPFALSE;
		break;

	case SpTransTypeOut:
		Invert = KPTRUE;
		break;

	case SpTransTypeSim:
		Status = SpXformGet (Profile, WhichRender, SpTransTypeOut, &Xforms [0]);
		if ((SpStatSuccess != Status) &&
		    (SpStatXformIsPerceptual != Status) &&
		    (SpStatXformIsColormetric != Status) &&
		    (SpStatXformIsSaturation != Status)) {
			return Status;
		}	

		/* if rendering intent was any, extract intent that was found */
		if (WhichRender == SpTransRenderAny) {
			XformData = SpXformLock (Xforms[0]);
			if (NULL == XformData) {
				return SpStatBadXform;
			}

			FoundRender = XformData->WhichRender;

			SpXformUnlock (Xforms[0]);
			XformData = NULL;
		}

/* Per ICC Spec Section E.8, simulation input should be 
   colorimetric rendering intent */
		Status = SpXformGet (Profile, SpTransRenderColormetric, 
				SpTransTypeIn, &Xforms [1]);
		if ((SpStatSuccess != Status) &&
		    (SpStatXformIsPerceptual != Status) &&
		    (SpStatXformIsColormetric != Status) &&
		    (SpStatXformIsSaturation != Status))
		{
			SpXformFree (&Xforms [0]);
			return Status;
		}

		Status = SpConnectSequenceEx (SpConnect_pf_16, 2, Xforms,
					Xform, &FailXform, NULL, NULL);

		if (SpStatSuccess == Status) {
			XformData = SpXformLock (*Xform);
			if (NULL == XformData) {
				return SpStatBadXform;
			}
			
			if (WhichRender == SpTransRenderAny) {
				XformData->WhichRender = FoundRender;
			}
			else {
				XformData->WhichRender = WhichRender;
			}

			XformData->WhichTransform = WhichTrans;

			SetWtPt(Profile, XformData);

			SpXformUnlock (*Xform);
		}
		
		SpXformFree (&Xforms [1]);
		SpXformFree (&Xforms [0]);
		return Status;
	
	case SpTransTypeGamut:
		return SpStatUnsupported;

	default:
		return SpStatOutOfRange;
	}
	
/* get the colorant tags */
	Status = SpTagGetById (Profile, SpTagRedColorant, &Value);
	if (SpStatSuccess == Status){

		rXYZ = Value.Data.XYZ;
		SpTagFree (&Value);

		Status = SpTagGetById (Profile, SpTagGreenColorant, &Value);
		if (SpStatSuccess != Status)
			return Status;

		gXYZ = Value.Data.XYZ;
		SpTagFree (&Value);

		Status = SpTagGetById (Profile, SpTagBlueColorant, &Value);
		if (SpStatSuccess != Status)
			return Status;

		bXYZ = Value.Data.XYZ;
		SpTagFree (&Value);

/* get the responce data */
		Status = SpTagGetById (Profile, SpTagRedTRC, &rTRC);
		if (SpStatSuccess != Status)
			return Status;

		Status = SpTagGetById (Profile, SpTagGreenTRC, &gTRC);
		if (SpStatSuccess != Status) {
			SpTagFree (&rTRC);
			return Status;
		}

		Status = SpTagGetById (Profile, SpTagBlueTRC, &bTRC);
		if (SpStatSuccess != Status) {
			SpTagFree (&rTRC);
			SpTagFree (&gTRC);
			return Status;
		}

/* call the LCSCreate function to generate the fut... */
		Status = SpXformCreate (&rXYZ, &gXYZ, &bXYZ,
					&rTRC.Data.Curve,
					&gTRC.Data.Curve,
					&bTRC.Data.Curve,
					GridSize, Invert, Adapt,
					Linear, Xform);

/* free the Response tags */
		SpTagFree (&rTRC);
		SpTagFree (&gTRC);
		SpTagFree (&bTRC);
	} else
	{
/* Check if Gray Scale Profile */
		Status = SpTagGetById (Profile, SpTagGrayTRC, &gTRC);
		if (SpStatSuccess == Status) {
			Status = SpXformGrayCreate(
					&gTRC.Data.Curve,
					GridSize, Invert, Xform);
			SpTagFree (&gTRC);
		}
	}

	if (SpStatSuccess == Status) {
		XformData = SpXformLock (*Xform);
		if (NULL == XformData) {
			return SpStatBadXform;
		}

		XformData->WhichRender = WhichRender;
		XformData->WhichTransform = WhichTrans;

		SetWtPt(Profile, XformData);

		SpXformUnlock (*Xform);
	}

	return Status;
}




#if defined (SP_WRITE_PTS)
/************* start debugging routines ****************/


static void getPTFileName(KpChar_p ptFileName)
{
	static KpInt32_t ptNum=1;	/* this is highly illegal in a dll, but it's only for testing !*/
	KpChar_t numString[12];

	strcpy(ptFileName, "sprof");
	KpItoa (ptNum, numString);
	strcat(ptFileName, numString);
	strcat(ptFileName, ".pt");
	ptNum++;
}

static void savePT(KpChar_p ptFileName, KpUInt32_t size, SpHugeBuffer_t ptData)
{
	int fileStatus;
	KpFileId fdp;

	fileStatus = KpFileOpen (ptFileName, "w", NULL, &fdp);
	if (fileStatus != KCMS_IO_SUCCESS) return;

	KpFileWrite(fdp, ptData, size);

	KpFileClose(fdp);

}
/************* end debugging routines ****************/
#endif

     
