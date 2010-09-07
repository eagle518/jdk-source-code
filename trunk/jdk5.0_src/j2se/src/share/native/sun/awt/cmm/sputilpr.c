/*
 * @(#)sputilpr.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the functions not needed
			by the SUN nor Java Libraries.

			Pulled from spsave.c, speval.c, splut.c, and
			sptagmgr.c 7/2/99

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, 
			all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sputilpr.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/sputilpr.c $
		$Revision: 12 $
		$Date: 9/24/02 10:57a $
		$Author: Arourke $

	SCCS Revision:
	 @(#)sputilpr.c	1.2	12/19/03

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

#include <stdio.h>
#include "sprof-pr.h"
#include <string.h>
#include "spevalcb.h"


#if !defined (SP_NO_OBSOLETE)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Private)
 *	Save a profile to a named file.  Optionally share the tag data.
 * 	This function has been obsoleted by SpProfileSaveProfileEx
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveToFileEx (
				SpProfile_t Profile,
				char		FAR *Name,
				KpBool_t		ShareTags)
{
	SpFileProps_t		Props;

/* Create SpFileProps_t and call SpProfileSaveProfileEx */
#if defined (KPMAC)
	strncpy (Props.fileType, "prof", 5);
	strncpy (Props.creatorType, "KCMM", 5);
	Props.vRefNum = 0;
	Props.dirID = 0;
#endif

	return (SpProfileSaveProfileEx(Profile, Name, &Props, ShareTags));
}


/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save a profile to a named file.  Optionally share the tag data.
 * 	This function has been obsoleted by SpProfileSaveProfileEx
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveToDiskEx (
				SpProfile_t		Profile,
				KpChar_t		*Name,
				SpIOFileChar_t	*Props,
				KpBool_t		ShareTags)
{
SpFileProps_t	*PropsPtr;
#if defined (KPMAC)
SpFileProps_t	SpProps;

/* Create SpFileProps_t and call SpProfileSaveProfileEx */
	if (Props != NULL) { 
		SpCvrtIOFileProps(Props, &SpProps);
		PropsPtr = &SpProps;
	}
	else
		PropsPtr = NULL;
#else
	if (Props) {}
	PropsPtr = NULL;
#endif

	return (SpProfileSaveProfileEx(Profile, Name, PropsPtr, ShareTags));
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save a profile to a named file.  Share the tag data.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveToDisk (
				SpProfile_t		Profile,
				KpChar_t		*Name,
				SpIOFileChar_t	*Props)
{
	return SpProfileSaveToDiskEx (Profile, Name, Props, KPTRUE);
}
#endif /* !SP_NO_OBSOLETE */

#if !defined (SP_NO_FILEIO)
/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save a profile to a named file.  Share the tag data.
 *
 * AUTHOR
 * 	acr
 *
 * DATE CREATED
 *	December 12, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveProfile (
				SpProfile_t		Profile,
				KpChar_t		*Name,
				SpFileProps_t	*Props)
{
	return SpProfileSaveProfileEx (Profile, Name, Props, KPTRUE);
}
#endif


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	copy profile header.
 *
 * AUTHOR
 * 	ae
 *
 * DATE CREATED
 *	January 13, 1994
 *------------------------------------------------------------------*/
static void SpProfileCopyHeader (
				char			*pBuffer,
				SpProfileData_t	*ProfileData)
{
	SpStatus_t	Status;
	char		Buffer [128];

	Status = SpHeaderFromPublic (&ProfileData->Header, sizeof (Buffer), Buffer);
	if (SpStatSuccess != Status) {
		return;
	}

	memcpy (pBuffer, Buffer, sizeof (Buffer));
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do padding to force tag data to a 4 byte boundry.
 *
 * AUTHOR
 * 	ae
 *
 * DATE CREATED
 *	January 13, 1997
 *------------------------------------------------------------------*/
static SpStatus_t DoBufferPadding (
				KpChar_t	**pBuffer,
				KpUInt32_t	*BufferSize)
{

/* determine where this tag will go in the buffer */
	if (0 != *BufferSize % 4) {
	
		*pBuffer += 4 - (*BufferSize % 4);
		*BufferSize += 4 - (*BufferSize % 4);
	}

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Save the tag directory to buffer .
 *
 * AUTHOR
 * 	ae
 *
 * DATE CREATED
 *	January 15, 1997
 *------------------------------------------------------------------*/

static void SpWriteTagDirToBuffer (
				char			**Buffer,
				KpUInt32_t		Count,
				SpTagRecord_t	FAR *TagRecords)
{
	KpUInt32_t		i;
	char			*Ptr;

	Ptr = *Buffer;

	SpPutUInt32 (&Ptr, Count);
	for (i = 0;
				i < Count;
						i++, TagRecords++) {
		SpPutUInt32 (&Ptr, TagRecords->Id);
		SpPutUInt32 (&Ptr, TagRecords->Offset);
		SpPutUInt32 (&Ptr, TagRecords->Size);
	}
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Get the profile size 
 *
 * AUTHOR
 * 	ae
 *
 * DATE CREATED
 *	january 13, 1997
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetProfileSize (
				SpProfile_t	Profile,
				KpUInt32_p	Size)
{
	SpProfileData_t	FAR *ProfileData;
	SpTagDirEntry_t	FAR *tagArray;
	KpInt32_t		index;
	KpInt32_t		activeCount = 0;
	KpUInt32_t		BufferPos;


/* lock down the profile data */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* determine the number of active TagArray elements */
	activeCount = SpTagGetCount (ProfileData);
	
	BufferPos = HEADER_SIZE + (activeCount * 12) + sizeof(KpUInt32_t);

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);


	for (index = 0; index < ProfileData->TotalCount; index++)
	{
		
	/* determine if this TagArray element holds good data */
		if (tagArray[index].TagDataSize == -1)
			continue;
				
	/* determine where this tag will go in the buffer */
		if (0 != BufferPos % 4) {
			BufferPos += 4 - (BufferPos % 4);
		}

		if (!SpTagShare(tagArray, index, NULL, NULL))
			BufferPos += tagArray[index].TagDataSize;

	}
	
	/* Profile Siz eis now to be a multiple of 4 */
	if (0 != BufferPos % 4) {
		BufferPos += 4 - (BufferPos % 4);
	}

	unlockBuffer (ProfileData->TagArray);

	SpProfileUnlock (Profile);

	*Size = BufferPos;

	return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Calculate the Profile ID
 *      
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	February 27, 2001
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetProfileId (
				SpProfile_t	Profile,
				KpUInt32_p	ID)
{
	SpProfileData_t	FAR *ProfileData;
	SpTagRecord_t	FAR *ArrayRecords;
	SpTagRecord_t	FAR *TagRecords;
	SpTagRecord_t	FAR *TagRecord;
	SpTagDirEntry_t	FAR *tagArray;
	void 			KPHUGE *tagData;
	KpInt32_t		index;
	KpInt32_t		activeCount = 0;
	KpUInt32_t		incrementTagRecord = 0;
	char			*TagDataBuf, *TagBuffer, *Buffer, *BytePtr;
	KpUInt32_t		BuffSize, bytes, TagSize;
	MD5_CTX			context;
	SpStatus_t		Status;

	BytePtr = 
	Buffer = SpMalloc(128);
	if (NULL == Buffer)
		return SpStatMemory;

	KpMemSet(Buffer, 0, 128);

	Status = SpProfileGetProfileSize(Profile, &BuffSize);

/* lock down the profile data */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* write the Header */
	SpProfileCopyHeader (Buffer, ProfileData);

/* Clear out flags, Rendering, and ProfileId  */
	/* Flags */
	BytePtr = Buffer + 44;
	SpPutUInt32(&BytePtr, 0);

	/* Rendering */
	BytePtr = Buffer + 64;
	SpPutUInt32(&BytePtr, 0);

	/* ProfileId */
	BytePtr = Buffer + 84;
	SpPutUInt32(&BytePtr, 0);
	SpPutUInt32(&BytePtr, 0);
	SpPutUInt32(&BytePtr, 0);
	SpPutUInt32(&BytePtr, 0);

/* determine the number of active TagArray elements */
	activeCount = SpTagGetCount (ProfileData);
	
/* allocate space for tag records */
	bytes = activeCount * sizeof (*TagRecords) + sizeof(KpUInt32_t);

	ArrayRecords = SpMalloc (bytes);
	if (NULL == ArrayRecords) {
		SpProfileUnlock (Profile);
		return SpStatMemory;
	}
/* zero out Tag Record before using */
	KpMemSet (ArrayRecords, 0, bytes);

	BytePtr = (char *)ArrayRecords;
	SpPutUInt32(&BytePtr, activeCount);
	TagRecords = TagRecord = (SpTagRecord_t *)BytePtr;
	
	TagSize = BuffSize - bytes - HEADER_SIZE;

	TagDataBuf = TagBuffer = SpMalloc(TagSize);
	if (NULL == TagDataBuf) {
		SpProfileUnlock (Profile);
		return SpStatMemory;
	}

/* zero out Tag buffer before using */
	KpMemSet (TagDataBuf, 0, TagSize);


/**********************/
/* write the Tag data */
/**********************/

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
		
	BuffSize = HEADER_SIZE + (activeCount * 12) + sizeof(KpUInt32_t);
	for (index = 0; index < ProfileData->TotalCount; index++)
	{

	/* determine if this TagArray element holds good data */
		if (tagArray[index].TagDataSize == -1)
			continue;
		else
			incrementTagRecord = 1;

	/* try to share data from previous tags */
		if (!SpTagShare (tagArray, index, TagRecords, TagRecord))
		{

	/* not sharing tag data, write data to file */

	/* remember where we put this tag */
			TagRecord->Id = (KpUInt32_t) tagArray[index].TagId;
			TagRecord->Offset = (KpUInt32_t) BuffSize;
			TagRecord->Size = tagArray[index].TagDataSize;

	/* write the tag to the file */
			tagData = (void KPHUGE *) lockBuffer (tagArray[index].TagData);
		
			memcpy (TagBuffer, tagData, tagArray[index].TagDataSize);

			BuffSize += tagArray[index].TagDataSize;
			TagBuffer += tagArray[index].TagDataSize;
			unlockBuffer (tagArray[index].TagData);

			/* determine where the next tag will go in the buffer */
			DoBufferPadding (&TagBuffer, &BuffSize);
		}

	/* point to the next tag directory entry */
		if (incrementTagRecord)
		{
			TagRecord++;
			incrementTagRecord = 0;

		}
	}

	unlockBuffer (ProfileData->TagArray);

	SpProfileUnlock (Profile);

/* Set the profile size */
	BytePtr = Buffer;
	SpPutUInt32(&BytePtr, BuffSize);

/* Start the MD5 calc with the header info */
	MD5Init(&context);

	MD5Update(&context, Buffer, 128);
	SpFree (Buffer);

/********************************/
/* Calc MD5 for the Tag Array */
/********************************/
#if defined (KPLSBFIRST)
			// byte swap the tag array data if need be.
	BytePtr = (char *)ArrayRecords;
	BytePtr += 4;	// tag count is already swapped
	Kp_swab32 ((KpGenericPtr_t)(BytePtr), bytes/4 - 1);
#endif
	MD5Update(&context, ArrayRecords, bytes);
	SpFree (ArrayRecords);

/********************************/
/* Calc MD5 for the Tag directory */
/********************************/
	MD5Update(&context, TagDataBuf, TagSize);
	SpFree (TagDataBuf);

/* Now get the return value */
	MD5Final((char *)ID, &context);

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save the profile data to the buffer.   
 *      
 *
 * AUTHOR
 * 	ae
 *
 * DATE CREATED
 *	January 13, 1997
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveToBuffer (
				SpProfile_t	Profile,
				KpChar_p	*lpBuffer,
				KpUInt32_p	inBytes)
{
	SpProfileData_t	FAR *ProfileData;
	SpTagRecord_t	FAR *TagRecords;
	SpTagRecord_t	FAR *TagRecord;
	SpTagDirEntry_t	FAR *tagArray;
	void 			KPHUGE *tagData;
	KpInt32_t		index;
	KpInt32_t		activeCount = 0;
	KpUInt32_t		incrementTagRecord = 0;
	char			*Buffer, *BytePtr;
	KpUInt32_t		BuffSize, inSize, bytes;
	KpUInt32_t		ProfID[4];
	SpStatus_t		Status;

	Buffer = *lpBuffer;
	inSize = *inBytes;
/* lock down the profile data */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;


/* determine the number of active TagArray elements */
	activeCount = SpTagGetCount (ProfileData);
	
/* allocate space for tag records */
	bytes = activeCount * sizeof (*TagRecords);
	TagRecord =
	TagRecords = SpMalloc (bytes);
	if (NULL == TagRecords) {
		SpProfileUnlock (Profile);
		return SpStatMemory;
	}
	
/* zero out TagRecord buffer before using */
	KpMemSet (TagRecord, 0, activeCount * sizeof (*TagRecords));

/* write the Header */
	SpProfileCopyHeader (Buffer, ProfileData);

	BuffSize = HEADER_SIZE + (activeCount * 12) + sizeof(KpUInt32_t);
	Buffer += BuffSize;

/**********************/
/* write the Tag data */
/**********************/

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
		
	for (index = 0; index < ProfileData->TotalCount; index++)
	{
		
	/* determine if this TagArray element holds good data */
		if (tagArray[index].TagDataSize == -1)
			continue;
		else
			incrementTagRecord = 1;
				
	/* determine where this tag will go in the buffer */
		DoBufferPadding (&Buffer, &BuffSize);

	/* try to share data from previous tags */
		if (!SpTagShare (tagArray, index, TagRecords, TagRecord))
		{

	/* not sharing tag data, write data to file */

	/* remember where we put this tag */
			TagRecord->Id = (KpUInt32_t) tagArray[index].TagId;
			TagRecord->Offset = (KpUInt32_t) BuffSize;
			TagRecord->Size = tagArray[index].TagDataSize;

	/* write the tag to the file */
			tagData = (void KPHUGE *) lockBuffer (tagArray[index].TagData);
		
			memcpy (Buffer, tagData, tagArray[index].TagDataSize);

			BuffSize += tagArray[index].TagDataSize;
			Buffer += tagArray[index].TagDataSize;
			unlockBuffer (tagArray[index].TagData);
		}

	/* point to the next tag directory entry */
		if (incrementTagRecord)
		{
			TagRecord++;
			incrementTagRecord = 0;
		}
	}
	
	/* Make the profile size a multiple of 4 */
	DoBufferPadding (&Buffer, &BuffSize);

	unlockBuffer (ProfileData->TagArray);

/*******************************/
/* update the size of the buffer */
/*******************************/
	BytePtr = *lpBuffer;
/*	*Size = BuffSize; */
	SpPutUInt32(&BytePtr, BuffSize);
	
/* Update the MD5 Profile ID if profile is newer */
	if (ProfileData->Header.ProfileVersion >= SpProfileTagVersion)
	{
		Status = SpProfileGetProfileId(Profile, ProfID);
		BytePtr = *lpBuffer + 84;
		memcpy (BytePtr, ProfID, 16);
	}


/********************************/
/* copy the Tag directory */
/********************************/
	BytePtr = *lpBuffer + HEADER_SIZE;
	SpWriteTagDirToBuffer(&BytePtr, activeCount, TagRecords);

/* done with tag records, free space */
	SpFree (TagRecords);
	SpProfileUnlock (Profile);

	return SpStatSuccess;
}



/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Connect transforms using default grid size.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	April 25, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpCombineXforms (
				KpInt32_t		XformCnt,
				SpXform_t		FAR *XformSequence,
				SpXform_t		FAR *Result,
				KpInt32_t		FAR *FailingXform,
				SpProgress_t	ProgressFunc,
				void			FAR *Data)
{
	return SpConnectSequenceEx (SpConnect_pf | SpConnect_Largest,
				XformCnt, XformSequence,
				Result, FailingXform, ProgressFunc, Data);
}


#include <string.h>
#include <stdio.h>
#include "sprof-pr.h"
#include "attrcipg.h"


/*--------------------------------------------------------------------
* DESCRIPTION
 *      Create a Transform with a PT from a Lut
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      December 27, 1995
 *------------------------------------------------------------------*/

SpStatus_t KSPAPI SpXformFromLut(SpLut_t		Lut,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				KpF15d16XYZ_t	HdrWhite,
				KpF15d16XYZ_t	MedWhite,
				KpUInt32_t	ChainIn,
				KpUInt32_t	ChainOut,
				SpXform_t	FAR *Xform)
{
SpStatus_t	Status;

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];
	sprintf (string, "\nSpXformFromLut\n");
	kcpDiagLog (string); }
	#endif

Status = SpXformFromLutDT(NO_DT_ICC_TYPE,
			  Lut,
			  WhichRender,
			  WhichTransform,
			  SpaceIn,
			  SpaceOut,
			  HdrWhite,
			  MedWhite,
			  ChainIn,
			  ChainOut,
			  Xform);

return Status;
}

/*--------------------------------------------------------------------
* DESCRIPTION
 *      Create a Transform with a PT from a Lut
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      December 27, 1995
 *------------------------------------------------------------------*/

SpStatus_t KSPAPI SpXformFromLutDT(
				KpInt32_t	SpDataType,
				SpLut_t		Lut,
				SpTransRender_t	WhichRender,
				SpTransType_t	WhichTransform,
				SpSig_t		SpaceIn,
				SpSig_t		SpaceOut,
				KpF15d16XYZ_t	HdrWhite,
				KpF15d16XYZ_t	MedWhite,
				KpUInt32_t	ChainIn,
				KpUInt32_t	ChainOut,
				SpXform_t	FAR *Xform)
{
void	KPHUGE	*Buf;
KpUInt32_t		BufSize;
SpStatus_t		Status;
SpXformData_t   FAR *XformData;
KpInt32_t		KcmDataType;

	Status = SpDTtoKcmDT (SpDataType, &KcmDataType);
	if (Status != SpStatSuccess) {
		return (Status);
	}

	/* Allocate the Space */
	Status = SpXformAllocate (Xform);
	if (SpStatSuccess != Status)
	{
		*Xform = NULL;
		return Status;
	}

	/* Set Up Private Structure to Populate */
	XformData = lockBuffer ((KcmHandle)*Xform);
	if (NULL == XformData)
	{
		SpXformFree (Xform);
		*Xform = NULL;
		return SpStatBadXform;
	}

	/* Signature is set in XfromAllocate */

	/* Check Lut Type for Legal Value */
	if (Lut.LutType == SpTypeLut8) {
		XformData->LutType        = SpTypeLut8;
		XformData->LutSize        = SP_ICC_MFT1;
	} else if (Lut.LutType == SpTypeLut16) {
		XformData->LutType        = SpTypeLut16;
		XformData->LutSize        = SP_ICC_MFT2;
	} else if (Lut.LutType == SpTypeLutBA) {
		XformData->LutType        = SpTypeLutBA;
		XformData->LutSize        = SP_ICC_MBA2;
		if ( Lut.L.LutAB.CLUT != 0 &&
			Lut.L.LutAB.CLUT->Precision == 1 ){
			XformData->LutSize        = SP_ICC_MBA1;
		}
	} else if (Lut.LutType == SpTypeLutAB) {
		XformData->LutType        = SpTypeLutAB;
		XformData->LutSize        = SP_ICC_MAB2;
		if ( Lut.L.LutAB.CLUT != 0 &&
			Lut.L.LutAB.CLUT->Precision == 1 ){
			XformData->LutSize        = SP_ICC_MAB1;
		}
	}

	/* Get the Buffer and Size to checking to the Color Processor */
	Status = SpLutFromPublic(&Lut, &BufSize, &Buf);
	if (SpStatSuccess != Status)
	{
		SpFree (XformData);
		return Status;
	}

	/* Get the PTRefNum for the Checked in Lut */
	Status = SpXformLoadImp(Buf, BufSize, 
				KcmDataType, SpaceIn, SpaceOut, 
				&XformData->PTRefNum);

	SpFree(Buf);

	/* Initialize the PT Chaining Values */
	if (SpStatSuccess == Status)
		Status = SpSetKcmAttrInt(XformData->PTRefNum,
					 KCM_OUT_CHAIN_CLASS_2,
					 ChainOut);
	if (SpStatSuccess == Status)
		Status = SpSetKcmAttrInt(XformData->PTRefNum,
					 KCM_IN_CHAIN_CLASS_2,
					 ChainIn);

	/* If something Failed, Free the Xform Space 
	   and report the error */
	if (SpStatSuccess != Status)
	{
		SpXformFree (Xform);
		*Xform = NULL;
		return Status;
	}

	/* Fill in the rest of the information */
	XformData->WhichRender    = WhichRender;
	XformData->WhichTransform = WhichTransform;
	XformData->SpaceIn        = SpaceIn;
	XformData->SpaceOut       = SpaceOut;
	XformData->ChainIn        = ChainIn;
	XformData->ChainOut       = ChainOut;
	XformData->HdrWtPoint.X   = HdrWhite.X;
	XformData->HdrWtPoint.Y   = HdrWhite.Y;
	XformData->HdrWtPoint.Z   = HdrWhite.Z;
	XformData->MedWtPoint.X   = MedWhite.X;
	XformData->MedWtPoint.Y   = MedWhite.Y;
	XformData->MedWtPoint.Z   = MedWhite.Z;
	if (HdrWhite.X + HdrWhite.Y + HdrWhite.Z > 0.0)
		XformData->HdrWPValid = KPTRUE;
	if (MedWhite.X + MedWhite.Y + MedWhite.Z > 0.0)
		XformData->MedWPValid = KPTRUE;

	#if defined KCP_DIAG_LOG
	{KpChar_t	string[256];

	sprintf (string, "\nSpXformFromLutDT\n");
	sprintf (string, " SpDataType %d, &Lut %x, WhichRender %d,",
			SpDataType, &Lut, WhichRender);
	sprintf (string, " WhichTransform %d, SpaceIn %4.4s, SpaceOut %4.4s",
			WhichTransform, &SpaceIn, &SpaceOut);
	sprintf (string, " ChainIn %d, ChainOut %d, *Xform %x",
			ChainIn, ChainOut, *Xform);
	sprintf (string, " XformData->PTRefNum %x\n",
			XformData->PTRefNum);
	kcpDiagLog (string); }
	#endif

	unlockBuffer ((KcmHandle)*Xform);
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get raw profile header data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 16, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpRawHeaderGet (
				SpProfile_t	Profile,
				KpUInt32_t	BufferSize,
				void		KPHUGE *Buffer)
{
	SpStatus_t	Status;
	SpHeader_t	Header;
	KpUInt32_t	ProfSize;
	char		KPHUGE *Ptr = Buffer;
	SpProfileData_t	FAR *ProfileData;

	Status = SpProfileGetHeader (Profile, &Header);

	if (SpStatSuccess == Status)
		Status = SpHeaderFromPublic (&Header, BufferSize, Buffer);

	if (SpStatSuccess != Status)
		return Status;

	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	if ((ProfileData->ProfChanged == KPTRUE) ||
	    (ProfileData->ProfileSize == 128))
		Status = SpProfileGetProfileSize(Profile, &ProfSize);
	else
		ProfSize = ProfileData->ProfileSize;

	if (SpStatSuccess == Status)
		SpPutUInt32(&Ptr, ProfSize);

	return Status;
	
} /* SpRawHeaderGet */



#if !defined (SP_READONLY_PROFILES)

static SpStatus_t convertMFT (SpProfile_t pf, SpTagId_t tagID, SpSig_t newOrg);

/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Set the profile to a new data type 
 *
 * AUTHOR
 * 	King F. Choi
 *
 * DATE CREATED
 *	November 5, 1998
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetDT (
				KpUInt32_t		DataType,
				SpProfile_t		Profile)
{
	SpHeader_t		hdr;
	SpSig_t			newOrg;
	KpInt16_t		convertFlag = 0;
	SpStatus_t		st;

	st = SpProfileGetHeader (Profile, &hdr);
	if (st != SpStatSuccess)
		return st;

	switch (DataType) {
	case SP_ICC_TYPE_0:
		st = SpStatNotImp;
		break;
	case SP_ICC_TYPE_1:
		newOrg = SpSigOrgKodak_Type_1;
		if ((hdr.Originator == SpSigOrgKodak1_Ver_0) ||
			(hdr.Originator == SpSigOrgKodak2_Ver_0))
			convertFlag = 1;
		st = SpStatSuccess;
		break;
	case SP_ICC_TYPE_CURRENT:
		newOrg = SpSigOrgKodak;
		if ((hdr.Originator == SpSigOrgKodak1_Ver_0) ||
			(hdr.Originator == SpSigOrgKodak2_Ver_0))
			convertFlag = 1;
		st = SpStatSuccess;
		break;
	default:
		st = SpStatNotImp;
		break;
	}

	if (convertFlag) {
		st = convertMFT (Profile, SpTagAToB0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagAToB1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagAToB2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagGamut, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		hdr.Originator = newOrg;
		st = SpProfileSetHeader (Profile, &hdr);
	}

	return st;
}


/*--------------------------------------------------------------------
 *	Subroutine:		convertMFT
 *	Function:		Convert profile to type 1 format
 *	Inputs:			pf - profile (SpProfile_t)
 *					Tag - profile tag (SpTagValue_t)
 *					newOrg - new originator (SpSig_t)
 *	Outputs:		none
 *	Return value:	none
 *	Author:			King F. Choi
 *	Date:			October 16, 1998
 *------------------------------------------------------------------*/
static SpStatus_t convertMFT (SpProfile_t pf, SpTagId_t tagID, SpSig_t newOrg)
{
	SpTagValue_t	tag;
	SpHeader_t		hdr;
	SpSig_t			oldOrg;
	SpTransRender_t	render = SpTransRenderPerceptual;
	SpTransType_t	transform = SpTransTypeIn;
	SpStatus_t		st;
	SpXform_t		xform;
	SpSig_t			LutType;

	st = SpTagGetById (pf, tagID, &tag);
	if (st != SpStatSuccess) {
		return 	SpStatSuccess;	/* assume no Lut of this type, 
					   nothing to do */
	}
	
	LutType = tag.Data.Lut.LutType;
	SpTagFree(&tag);
	
	if (LutType != SpTypeLut16) {
		return 	SpStatSuccess;	/* only convert Lut16 */
	}
	
	switch ((KpInt32_t)tagID) {
	case (KpInt32_t)SpTagAToB0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagAToB1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagAToB2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagBToA0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeOut;
		break;
		
	case (KpInt32_t)SpTagBToA1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeOut;
		break;
		
	case (KpInt32_t)SpTagBToA2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeOut;
		break;

	case (KpInt32_t)SpTagGamut:
		render = SpTransRenderAny;
		transform = SpTransTypeGamut;
		break;

	case (KpInt32_t)SpTagPreview0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeSim;
		break;

	case (KpInt32_t)SpTagPreview1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeSim;
		break;

	case (KpInt32_t)SpTagPreview2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeSim;
		break;

	default:
		st = SpStatOutOfRange;
		return st;
	}
	
	SpXformGet (pf, render, transform, &xform);
	st = SpProfileGetHeader (pf, &hdr);
	if (st == SpStatSuccess) {
		oldOrg = hdr.Originator;
	    hdr.Originator = newOrg;
		st = SpProfileSetHeader (pf, &hdr);
	}
	if (st == SpStatSuccess) {
		SpXformSet (pf, 16, render, transform, xform);
	    hdr.Originator = oldOrg;
		st = SpProfileSetHeader (pf, &hdr);
	}
	SpXformFree(&xform);
	return st;
}


#endif


     
     
