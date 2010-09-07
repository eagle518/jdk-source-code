/*
 * @(#)spxfprv.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/* @(#)spxfprv.c	1.89 99/02/16
	Contains:	This module contains the transform functions
			not needed by SUN nor Java Libraries.

			Pulled from spxform.c and spxfromr.c 7/2/99

	Written by:	The Kodak CMS Team

	COPYRIGHT (c) Eastman Kodak Company, 1993-1999
	As an unpublished work pursuant to Title 17 of the United
	States Code.  All rights reserved.
*/

#include "sprof-pr.h"
#include <string.h>
#include <stdio.h>
#include "attrcipg.h"


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Set the color space attribute for a Color Processor Transform.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 24, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpXformSetColorSpace (
				PTRefNum_t	RefNum,
				KpInt32_t	AttrNum,
				KpInt32_t	SpColorSpace)
{
	KpInt32_t	AttrValue;
	SpStatus_t	Status;

	Status = SpColorSpaceSp2Kp (SpColorSpace, &AttrValue);
	if (Status == SpStatOutOfRange)
		/* must be unknown color space - set ICC strings to CP*/
		SpSetColorSpaceICC2Kp (RefNum, AttrNum, SpColorSpace);

	return SpSetKcmAttrInt (RefNum, AttrNum, AttrValue);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create a duplicate of an existing transform.
 *
 * AUTHOR
 * 	gbp
 *
 * DATE Modified
 *	25 Jan 99
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformDuplicate (
				SpXform_t 	srcXform, 
				SpXform_t	FAR *dupXform)
{
	SpStatus_t		Status;
	PTRefNum_t		srcRefNum, dupRefNum;
	PTErr_t			PTStat;

	*dupXform = NULL;

/* Get the PTRefNum for the transform */
	Status = SpXformGetRefNum (srcXform, &srcRefNum);
	if (SpStatSuccess != Status) {
		return Status;
	}

/* duplicate it by using PTCombine */
	PTStat = PTCombine (0, srcRefNum, NULL, &dupRefNum);
	if (KCP_SUCCESS != PTStat) {
		return (SpStatusFromPTErr(PTStat));
	}

/* build an SpXform_t from the RefNum */
	if (SpStatSuccess == Status)
		Status = SpXformFromPTRefNumImp (dupRefNum, dupXform);

/* free PT if SpXform_t not created */
	if (SpStatSuccess != Status)
		PTCheckOut (dupRefNum);

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformFromBufferDT\n *dupXform %x\n",*dupXform);
	kcpDiagLog (string); }
	#endif

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the Associated Xform Data
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 10, 1996
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformGetParms (
				SpXform_t		Xform,
				SpTransRender_t		*WhichRender,
				SpTransType_t		*WhichTransform,
				KpF15d16XYZ_t		*HdrWhite,
				KpF15d16XYZ_t		*MedWhite,
				KpUInt32_t		*ChainIn,
				KpUInt32_t		*ChainOut)
{
	SpXformData_t		*XformData;

	XformData = SpXformLock(Xform);
	if (XformData == NULL)
		return(SpStatBadXform);

/* get the transform supplied data */
	*WhichRender	= XformData->WhichRender;
	*WhichTransform	= XformData->WhichTransform;

	if (XformData->HdrWPValid == KPTRUE)
	{
		HdrWhite->X	= XformData->HdrWtPoint.X;
		HdrWhite->Y	= XformData->HdrWtPoint.Y;
		HdrWhite->Z	= XformData->HdrWtPoint.Z;
	}
	else
	{
		HdrWhite->X	= 0;
		HdrWhite->Y	= 0;
		HdrWhite->Z	= 0;
	}

	if (XformData->MedWPValid == KPTRUE)
	{
		MedWhite->X	= XformData->MedWtPoint.X;
		MedWhite->Y	= XformData->MedWtPoint.Y;
		MedWhite->Z	= XformData->MedWtPoint.Z;
	}
	else 
	{
		MedWhite->X	= 0;
		MedWhite->Y	= 0;
		MedWhite->Z	= 0;
	}

	*ChainIn	= XformData->ChainIn;
	*ChainOut	= XformData->ChainOut;

	SpXformUnlock (Xform);
	return SpStatSuccess;

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Fill in the rest of the xform info
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 10, 1996
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformSetParms (
				SpXform_t		Xform,
				SpTransRender_t		WhichRender,
				SpTransType_t		WhichTransform,
				KpF15d16XYZ_t		HdrWhite,
				KpF15d16XYZ_t		MedWhite,
				KpUInt32_t		ChainIn,
				KpUInt32_t		ChainOut)
{
	SpXformData_t		*XformData;

	XformData = SpXformLock(Xform);
	if (XformData == NULL)
		return(SpStatBadXform);

/* give transform the supplied data */
	XformData->WhichRender    = WhichRender;
	XformData->WhichTransform = WhichTransform;

	XformData->HdrWtPoint.X   = HdrWhite.X;
	XformData->HdrWtPoint.Y   = HdrWhite.Y;
	XformData->HdrWtPoint.Z   = HdrWhite.Z;
	if (HdrWhite.X + HdrWhite.Y + HdrWhite.Z > 0)
		XformData->HdrWPValid = KPTRUE;
	XformData->MedWtPoint.X   = MedWhite.X;
	XformData->MedWtPoint.Y   = MedWhite.Y;
	XformData->MedWtPoint.Z   = MedWhite.Z;
	if (MedWhite.X + MedWhite.Y + MedWhite.Z > 0)
		XformData->MedWPValid = KPTRUE;

	XformData->ChainIn  = ChainIn;
	XformData->ChainOut = ChainOut;

	if (XformData->PTRefNum != NULL)
	{
		SpSetKcmAttrInt(XformData->PTRefNum,
			KCM_IN_CHAIN_CLASS_2, ChainIn);
		SpSetKcmAttrInt(XformData->PTRefNum,
			KCM_OUT_CHAIN_CLASS_2, ChainOut);
	}

	SpXformUnlock (Xform);
	return SpStatSuccess;

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get the number of input and output channels for a Transform.
 *
 * AUTHOR
 * 	mjb
 *
 * DATE CREATED
 *	October 17, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformGetChannels (
				SpXform_t		Xform,
				KpInt32_t		FAR *In,
				KpInt32_t		FAR *Out)
{

	PTRefNum_t	refNum;
	KpChar_t	attribute [KCM_MAX_ATTRIB_VALUE_LENGTH+1];
	KpInt32_t	inChans, outChans, temp, size, attribId, index;
	SpStatus_t	spStatus;
	PTErr_t		ptErr;

	/*	Get the PTRefNum for the transform	*/
	spStatus = SpXformGetRefNum (Xform, &refNum);
	if (SpStatSuccess != spStatus) {
		return spStatus;
	}

	/*	Start by determining the number of output channels	*/
	size = sizeof (attribute);
	ptErr = PTGetAttribute (refNum, KCM_NUM_VAR_OUT, &size, attribute);
	if (KCP_SUCCESS != ptErr) {
		return SpStatBadXform;
	}
	outChans = KpAtoi (attribute);

	/*	Check that we didn't exceed the maximum output channels	*/
	if (outChans > SpMaxComponents) {
		return SpStatBadXform;
	}

	/*	For each output channel get the number of input channels.
		The number of input channels must be the same for each 
		output channel.	*/
	inChans = -1;
	for (index = 0, attribId = KCM_NUM_VAR_1_IN; index < outChans; 
			index++, attribId++) {

		size = sizeof (attribute);
		ptErr = PTGetAttribute (refNum, attribId, &size, attribute);
		if (KCP_SUCCESS != ptErr) {
			return SpStatBadXform;
		}
		temp = KpAtoi (attribute);

		if (-1 == inChans) {
			inChans = temp;
		}
		else {
			if (temp != inChans) {
				return SpStatBadXform;
			}
		}

	}

	/*	return the number of input and output channels	*/
	*In = inChans;
	*Out = outChans;
	return SpStatSuccess;

}



/*--------------------------------------------------------------------
 * DESCRIPTION
 *      Create a Lut from an Xform
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      December 27, 1995
 *------------------------------------------------------------------*/

SpStatus_t KSPAPI SpXformToLut  (SpXform_t	Xform,
				SpLut_t		*Lut,
				SpTransRender_t	*WhichRender,
				SpTransType_t	*WhichTransform,
				SpSig_t		*SpaceIn,
				SpSig_t		*SpaceOut,
				KpF15d16XYZ_t	*HdrWhite,
				KpF15d16XYZ_t	*MedWhite,
				KpUInt32_t	*ChainIn,
				KpUInt32_t	*ChainOut)
{

	SpStatus_t	Status;

	Status = SpXformToLutDT(Xform, NO_DT_ICC_TYPE, Lut,
				WhichRender, WhichTransform,
				SpaceIn, SpaceOut,
				HdrWhite, MedWhite,
				ChainIn, ChainOut);
	return Status;
}

SpStatus_t KSPAPI SpXformToLutDT(SpXform_t	Xform,
				KpInt32_t	SpDataType,
				SpLut_t		*Lut,
				SpTransRender_t	*WhichRender,
				SpTransType_t	*WhichTransform,
				SpSig_t		*SpaceIn,
				SpSig_t		*SpaceOut,
				KpF15d16XYZ_t	*HdrWhite,
				KpF15d16XYZ_t	*MedWhite,
				KpUInt32_t	*ChainIn,
				KpUInt32_t	*ChainOut)
{
SpHugeBuffer_t	Buf;
char			*LutBuf;
KpUInt32_t		BufSize;
SpStatus_t		Status;
SpXformData_t   FAR *XformData;
KpInt32_t		KcmDataType;

	Status = SpDTtoKcmDT (SpDataType, &KcmDataType);
	if (Status != SpStatSuccess) {
		return (Status);
	}

	/* Set Up Private Structure to Populate */
	XformData = lockBuffer ((KcmHandle)Xform);
	if (NULL == XformData)
		return SpStatBadXform;

	/* Set Up Lut Size based on Lut Type and size */
	switch ( XformData->LutType )
	{	
	case SpTypeLut8:
		XformData->LutSize =  SP_ICC_MFT1;
		break;
	case SpTypeLut16:
		XformData->LutSize = SP_ICC_MFT2;
		break;
	case SpTypeLutAB:
		if ((XformData->LutSize == 0) ||
		    (XformData->LutSize == 16))
			XformData->LutSize = SP_ICC_MAB2;
		else
			XformData->LutSize = SP_ICC_MAB1;
		break;
	case SpTypeLutBA:
		if ((XformData->LutSize == 0) ||
		    (XformData->LutSize == 16))
			XformData->LutSize = SP_ICC_MBA2;
		else
			XformData->LutSize = SP_ICC_MBA1;
		break;
	} /* end switch */

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformToLutDT\n Xform %x, PTRefNum %x, Lut %x", 
		Xform, XformData->PTRefNum, Lut);
	kcpDiagLog (string); }
	#endif

	Status = SpXformGetDataFromCP  (XformData->PTRefNum,
					XformData->LutSize,
					KcmDataType,
					&BufSize,
					&Buf);

	if (Status != SpStatSuccess)
	{
		unlockBuffer ((KcmHandle)Xform);
		return Status;
	}

	Lut->LutType = XformData->LutType;

	/* Move past the tag and Reserved */
	LutBuf = (char *)Buf;
	LutBuf += 8;
	Status = SpLutToPublic(LutBuf, Lut);

	/* Clear White Points */
	HdrWhite->X = MedWhite->X =
	HdrWhite->Y = MedWhite->Y =
	HdrWhite->Z = MedWhite->Z = 0;

	if (Status == SpStatSuccess)
	{
		*WhichRender     = XformData->WhichRender;
		*WhichTransform  = XformData->WhichTransform;
		*SpaceIn         = XformData->SpaceIn;
		*SpaceOut        = XformData->SpaceOut;
		if (XformData->HdrWPValid) {
			HdrWhite->X       = XformData->HdrWtPoint.X;
			HdrWhite->Y       = XformData->HdrWtPoint.Y;
			HdrWhite->Z       = XformData->HdrWtPoint.Z;
		}
		if (XformData->MedWPValid) {
			MedWhite->X      = XformData->MedWtPoint.X;
			MedWhite->Y      = XformData->MedWtPoint.Y;
			MedWhite->Z      = XformData->MedWtPoint.Z;
		}
		*ChainIn         = XformData->ChainIn;
		*ChainOut        = XformData->ChainOut;
	}
	SpFree(Buf);
	unlockBuffer ((KcmHandle)Xform);
	return Status;
}

/***************************************************************************
 * FUNCTION NAME
 *      SpXformToPT
 *
 * DESCRIPTION
 *      This function copies the PT associated with the PTRefNum in
 *      the Xform to the Address given.  It will be in the format
 *      requested by the caller.  The SpXform is NOT freed by this
 *      function.
 *
 *
 
***************************************************************************/
SpStatus_t KSPAPI SpXformToPT(SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpUInt32_t	Datasize,
				SpHugeBuffer_t pXformData)
{
 
SpStatus_t	spStatus;

	spStatus = SpXformToBufferDT(spXform, LutType,
				NO_DT_ICC_TYPE,
				Datasize, pXformData);

	return spStatus;

}

/***************************************************************************
 * FUNCTION NAME
 *      SpXformBufferDT
 *
 * DESCRIPTION
 *      This function copies the PT associated with the PTRefNum in
 *      the Xform to the Address given.  It will be in the format
 *      requested by the caller.  The SpXform is NOT freed by this
 *      function.
 *
 *
 
***************************************************************************/
SpStatus_t KSPAPI SpXformToBufferDT(SpXform_t	spXform,
				KpUInt32_t	LutType,
				KpInt32_t	SpDataType,
				KpUInt32_t	Datasize,
				SpHugeBuffer_t pXformData)
{

PTRefNum_t	refNum;
SpStatus_t	spStatus;
KpUInt32_t	PTDatasize;
PTErr_t		PTStat;
KpUInt32_t	MFType;
KpInt32_t	KcmDataType;

	spStatus = SpDTtoKcmDT (SpDataType, &KcmDataType);
	if (spStatus != SpStatSuccess) {
		return (spStatus);
	}

	/*      Get the Color Processor PTRefNum of the SpXform */
	spStatus = SpXformGetRefNum (spXform, &refNum);
	if (SpStatSuccess != spStatus)
		return spStatus;

/* determine type of transform to get */
	switch (LutType) {
	case SP_ICC_FUT:
		MFType = PTTYPE_FUTF;
		break;

	case SP_ICC_MFT1:
		MFType = PTTYPE_MFT1;
		break;

	case SP_ICC_MFT2:
		if (KcmDataType == KCM_ICC_TYPE_0)
			MFType = PTTYPE_MFT2_VER_0;
		else
			MFType = PTTYPE_MFT2;
		break;

	case SP_ICC_MAB1:
		MFType = PTTYPE_MAB1;
		break;

	case SP_ICC_MAB2:
		MFType = PTTYPE_MAB2;
		break;

	case SP_ICC_MBA1:
		MFType = PTTYPE_MBA1;
		break;

	case SP_ICC_MBA2:
		MFType = PTTYPE_MBA2;
		break;

	default:
		return SpStatOutOfRange;
	}

/* ask color processor for the size of the Transform */
	PTStat = PTGetSizeF (refNum, MFType, (KpInt32_t FAR *)&PTDatasize);
	if (KCP_SUCCESS != PTStat)
		return SpStatusFromPTErr(PTStat);

	if (PTDatasize > Datasize)
		return SpStatBufferTooSmall;

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformToBufferDT\n spXform %x, PTRefNum %x", 
		spXform, pXformData);
	kcpDiagLog (string); }
	#endif

/* ask color processor for the Transform data */
	PTStat = PTGetPTF(refNum, MFType, 
			Datasize, (PTAddr_t) pXformData);

	return SpStatusFromPTErr(PTStat);

}

SpStatus_t KSPAPI SpXformGetPTSize(SpXform_t   spXform,
				KpUInt32_t	LutType,
				KpUInt32_t	*Datasize)
{
SpStatus_t	spStatus;

	spStatus = SpXformGetBufferSizeDT(spXform, LutType,
				NO_DT_ICC_TYPE,
				Datasize);

	return spStatus;

}

SpStatus_t KSPAPI SpXformGetBufferSizeDT(SpXform_t   spXform,
				KpUInt32_t	LutType,
				KpInt32_t	SpDataType,
				KpUInt32_t	*Datasize)
{
PTRefNum_t	refNum;
SpStatus_t	spStatus;
PTErr_t		PTStat;
KpUInt32_t	MFType;
KpInt32_t	KcmDataType;

	spStatus = SpDTtoKcmDT (SpDataType, &KcmDataType);
	if (spStatus != SpStatSuccess) {
		return (spStatus);
	}

	/*      Get the Color Processor PTRefNum of the SpXform */
	spStatus = SpXformGetRefNum (spXform, &refNum);
	if (SpStatSuccess != spStatus)
		return spStatus;

/* determine type of transform to get */
	switch (LutType) {
	case SP_ICC_FUT:
		MFType = PTTYPE_FUTF;
		break;

	case SP_ICC_MFT1:
		MFType = PTTYPE_MFT1;
		break;

	case SP_ICC_MFT2:
		if (KcmDataType == KCM_ICC_TYPE_0)
			MFType = PTTYPE_MFT2_VER_0;
		else
			MFType = PTTYPE_MFT2;
		break;

	case SP_ICC_MAB1:
		MFType = PTTYPE_MAB1;
		break;

	case SP_ICC_MAB2:
		MFType = PTTYPE_MAB2;
		break;

	case SP_ICC_MBA1:
		MFType = PTTYPE_MBA1;
		break;

	case SP_ICC_MBA2:
		MFType = PTTYPE_MBA2;
		break;

	default:
		return SpStatOutOfRange;
	}

	PTStat = PTGetSizeF (refNum, MFType, 
			(KpInt32_t FAR *) Datasize);
 
	return SpStatusFromPTErr(PTStat);
}

/***************************************************************************
 * FUNCTION NAME
 *	SpXformInvert
 *
 * DESCRIPTION
 *	This function inverts the transform specified by Xform.  If invertInp
 *	is true the input tables are inverted and if invertOut is true then
 *	the output tables are inverted.  
 *
 *	This function should be in the profile processor.  It was added here
 *	temporarily because the profile processor code was frozen at the time
 *	this function was created.
 *
 ***************************************************************************/
SpStatus_t KSPAPI SpXformInvert (
				SpXform_t Xform, 
				KpBool_t invertInp, 
				KpBool_t invertOut)
{

	PTRefNum_t		refNum;
	PTErr_t			ok;
	SpStatus_t		spStatus;

	/*	Get the PTRefNum for the transform	*/
	spStatus = SpXformGetRefNum (Xform, &refNum);
	if (SpStatSuccess != spStatus) {
		return spStatus;
	}

	/*	Invert the input tables	*/
	if (invertInp) {

		/* Set the KCM_MEDIUM_SENSE_IN attribute to KCM_POSITIVE, 
		   and the KCM_SENSE_INVERTIBLE_IN attribute to 
		   KCM_IS_INVERTIBLE			*/
		spStatus = SpSetKcmAttrInt (refNum, KCM_SENSE_INVERTIBLE_IN,
						KCM_IS_INVERTIBLE);  
		if (SpStatSuccess != spStatus) {
			return spStatus;
		}
		spStatus = SpSetKcmAttrInt (refNum, KCM_MEDIUM_SENSE_IN,
						KCM_POSITIVE);  
		if (SpStatSuccess != spStatus) {
			return spStatus;
		}
		ok = PTInvert (refNum, KCM_MEDIUM_SENSE_IN);
		if (ok != KCP_SUCCESS) {
			return SpStatusFromPTErr(ok);
		}
	}

	/*	Invert the output tables */
	if (invertOut) {

		/* Set the KCM_MEDIUM_SENSE_OUT attribute to KCM_POSITIVE, 
		   and the KCM_SENSE_INVERTIBLE_OUT attribute to 
		   KCM_IS_INVERTIBLE			*/
		spStatus = SpSetKcmAttrInt (refNum, KCM_SENSE_INVERTIBLE_OUT,
						KCM_IS_INVERTIBLE);  
		if (SpStatSuccess != spStatus) {
			return spStatus;
		}
		spStatus = SpSetKcmAttrInt (refNum, KCM_MEDIUM_SENSE_OUT,
						KCM_POSITIVE);  
		if (SpStatSuccess != spStatus) {
			return spStatus;
		}
		ok = PTInvert (refNum, KCM_MEDIUM_SENSE_OUT);
		if (ok != KCP_SUCCESS) {
			return SpStatusFromPTErr(ok);
		}
	}
	return SpStatSuccess;

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Construct Xform given a PT reference number.
 *	FOR PT2PF building ONLY !!
 *
 * AUTHOR
 * 	lcc
 *
 * DATE CREATED
 *	February 9, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformFromPTRefNumCombine (
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
		Status = SpXformBuildCnvrt (KPTRUE, Render, ConnectType, 
					ParadigmType, &CvrtInRefNum);
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
						ConnectType, ParadigmType, 
						&CvrtOutRefNum);

		if (SpStatSuccess == Status)
			RefNumList [Index++] = CvrtOutRefNum;
	}

/* fix the color spaces */
	if ((SpStatSuccess == Status) && (1 != Index)) {
		Status = SpConnectSequenceCombine (ConnectType, Index, 
						RefNumList, &NewRefNum, 
						&FailXform, NULL, NULL);
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
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_IN, 
					KCM_POSITIVE);
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
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_OUT, 
					KCM_POSITIVE);
		if (SpStatSuccess != Status) {
			PTCheckOut (*RefNum);
			return Status;
		}
	}

	Status = SpSetKcmAttrInt (*RefNum, KCM_CLASS, Class);
	Status = SpXformFromPTRefNumImp (*RefNum, Xform);
	*RefNum = 0;

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Construct Xform given a PT reference number using default grid size.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	June 17, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformFromPTRefNum (
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform)
{
	return SpXformFromPTRefNumEx (SpGetConnectType (), SpParadigmRel, 
					RefNum, Xform);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Construct Xform given a PT reference number.  No RCS -> LAB color
 *	space conversion is performed.
 *
 * AUTHOR
 * 	mjb
 *
 * DATE CREATED
 *	September 27, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformFromPTRefNumNC (
				PTRefNum_t		FAR *RefNum,
				SpXform_t		FAR *Xform)
{
	KpInt32_t	SpaceIn, SpaceOut;
	KpInt32_t	SenseIn, SenseOut;
	KpInt32_t	Class;
	int			Index;
	SpStatus_t	Status;
	PTErr_t		InvertStat;

	Class = SpGetKcmAttrInt (*RefNum, KCM_CLASS);
	SpaceIn = SpGetKcmAttrInt (*RefNum, KCM_SPACE_IN);
	SpaceOut = SpGetKcmAttrInt (*RefNum, KCM_SPACE_OUT);
	SenseIn = SpGetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_IN);
	SenseOut = SpGetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_OUT);

	Index = 0;
	Status = SpStatSuccess;

/* Invert input, if negative */
	if ((KCM_RCS != SpaceIn) && (KCM_NEGATIVE == SenseIn)) {
		InvertStat = PTInvert (*RefNum, KCM_MEDIUM_SENSE_IN);
		if (InvertStat != KCP_SUCCESS) {
			PTCheckOut (*RefNum);
			*RefNum = 0;
			return SpStatusFromPTErr(InvertStat);
		}
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_IN, 
					KCM_POSITIVE);
		if (SpStatSuccess != Status) {
			PTCheckOut (*RefNum);
			*RefNum = 0;
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
		Status = SpSetKcmAttrInt (*RefNum, KCM_MEDIUM_SENSE_OUT, 
					KCM_POSITIVE);
		if (SpStatSuccess != Status) {
			PTCheckOut (*RefNum);
			*RefNum = 0;
			return Status;
		}
	}

	Status = SpSetKcmAttrInt (*RefNum, KCM_CLASS, Class);
	if (SpStatSuccess != Status) {
		PTCheckOut (*RefNum);
		*RefNum = 0;
		return Status;
	}
	Status = SpXformFromPTRefNumImp (*RefNum, Xform);

	*RefNum = 0;

	return Status;

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create an SpXform_t from a data block containing a PT using the
 *	default grid size.  There is no RCS->LAB color space conversion.
 *	Used for importing the raw PT.
 *
 * AUTHOR
 * 	mjb
 *
 * DATE CREATED
 *	September 27, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpXformCreateFromDataNC (
				KpInt32_t	Size,
				KpLargeBuffer_t	Data,
				SpXform_t	FAR *Xform)
{

	SpStatus_t	Status;
	PTRefNum_t	RefNum;

	*Xform = NULL;

	Status = SpXformLoadImp (Data, Size, NO_DT_ICC_TYPE, SpSigNone, 
				SpSigNone, &RefNum);
	if (SpStatSuccess != Status)
		return Status;

	Status = SpXformFromPTRefNumNC (&RefNum, Xform);

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformCreateFromDataEx\n *Xform %x\n",*Xform);
	kcpDiagLog (string); }
	#endif

	return Status;
}

     
