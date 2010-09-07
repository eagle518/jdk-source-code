/*
 * @(#)spsave.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*********************************************************************/
/*
	Contains:	This module contains the profile tag manager.

				Created by lsh, October 18, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1997 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spsave.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spsave.c $
		$Revision: 6 $
		$Date: 2/19/02 1:37p $
		$Author: Doro $

	SCCS Revision:
	 @(#)spsave.c	1.39 1/11/99	

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
#include <stdio.h>

#include "sprof-pr.h"
#include <string.h>

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Write a 4 byte number to the file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 20, 1993
 *------------------------------------------------------------------*/
static void SpWriteUInt32 (
				KpBool_t		FAR *fOk,
				KpFileId	fd,
				KpUInt32_t	Value)
{
	KpInt32_t	NumBytes;
	char		Bytes [4];

	if (!fOk)
		return;

	Bytes [0] = (char) (Value >> 24);
	Bytes [1] = (char) (Value >> 16);
	Bytes [2] = (char) (Value >> 8);
	Bytes [3] = (char) (Value);

	NumBytes = 4;
	if (0 == KpFileWrite (fd, Bytes, NumBytes))
		*fOk = KPFALSE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do padding to force tag data to a 4 byte boundry.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 20, 1993
 *------------------------------------------------------------------*/
static SpStatus_t DoFilePadding (
				KpFileId	fd,
				KpInt32_t	FAR *FilePos)
{
	char			Padding [4];

/* determine where this tag will go in the file */
	KpFileTell (fd, FilePos);
	if (0 != *FilePos % 4) {

	/* clear array used for padding */
		KpMemSet (Padding, 0, sizeof (Padding));

	/* write padding bytes to force 32 bit alignment */
		if (0 == KpFileWrite (fd, Padding, 4 - (*FilePos % 4)))
			return SpStatFileWriteError;

		KpFileTell (fd, FilePos);
	}

	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Write profile header.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	Feb 14, 1994
 *------------------------------------------------------------------*/
static void SpProfileWriteHeader (
				KpBool_t			FAR *fOk,
				KpFileId		fd,
				SpProfileData_t	FAR *ProfileData)
{
	SpStatus_t	Status;
	char		Buffer [128];

	if (!*fOk)
		return;

	Status = SpHeaderFromPublic (&ProfileData->Header, sizeof (Buffer), Buffer);
	if (SpStatSuccess != Status) {
		*fOk = KPFALSE;
		return;
	}

	if (0 == KpFileWrite (fd, Buffer, sizeof (Buffer)))
		*fOk = KPFALSE;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Save the tag directory.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	March 15, 1994
 *------------------------------------------------------------------*/

static void SpWriteTagDir (
				KpBool_t			FAR *fOk,
				KpFileId		fd,
				KpUInt32_t		Count,
				SpTagRecord_t	FAR *TagRecords)
{
	KpUInt32_t		i;

	SpWriteUInt32 (fOk, fd, Count);
	for (i = 0;
				i < Count;
						i++, TagRecords++) {
		SpWriteUInt32 (fOk, fd, TagRecords->Id);
		SpWriteUInt32 (fOk, fd, TagRecords->Offset);
		SpWriteUInt32 (fOk, fd, TagRecords->Size);
	}
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Try to share data from previous tag.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	June 23, 1994
 *------------------------------------------------------------------*/
KpBool_t SpTagShare (
				SpTagDirEntry_t	FAR *TagArray,
				KpUInt32_t		Index,
				SpTagRecord_t	FAR *TagRecords,
				SpTagRecord_t	FAR *TagRecord)
{
	KpUInt32_t			index;
	SpTagRecord_t	FAR *TagCurrRecord;
	KpGenericPtr_t	FAR TagData1, FAR TagData2;

/* loop through previous tags looking for duplicate data */
	for (index = 0, TagCurrRecord = TagRecords;
			index != Index;
			index++)
	{
	
		/* compare the size of the data */
		if (TagArray[index].TagDataSize == TagArray[Index].TagDataSize) {

			TagData1 = lockBuffer(TagArray[index].TagData);
			TagData2 = lockBuffer(TagArray[Index].TagData);
	
			/* compare the actual data */
			if (0 == KpMemCmp (TagData1, TagData2,
							TagArray[Index].TagDataSize)) {

				/* data is the same, share this entry */
				if (TagRecord != NULL) {
					TagRecord->Id = (KpUInt32_t) 
						TagArray[Index].TagId;
					TagRecord->Offset = 
						TagCurrRecord->Offset;
					TagRecord->Size = TagCurrRecord->Size;
				}

				unlockBuffer(TagArray[index].TagData);
				unlockBuffer(TagArray[Index].TagData);
				return KPTRUE;
			}
		
			unlockBuffer(TagArray[index].TagData);
			unlockBuffer(TagArray[Index].TagData);
		}
		
		/* increment pointer into the new profile tag table if this is a valid
			entry in the sprofile Tag Array */
		if ((TagArray[index].TagDataSize != -1) &&
		    (TagRecord != NULL))
			TagCurrRecord++;
	}

	return KPFALSE;
}


#if !defined (SP_NO_OBSOLETE)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Private)
 *	Save a profile to a named file.  Share the tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveToFile (
				SpProfile_t Profile,
				char		FAR *Name)
{
SpFileProps_t	Props;

/* Create SpFileProps_t and call SpProfileSaveProfileEx */
#if defined (KPMAC)
	strncpy (Props.fileType, "prof", 5);
	strncpy (Props.creatorType, "KCMM", 5);
	Props.vRefNum = 0;
	Props.dirID = 0;
#endif

	return SpProfileSaveProfileEx (Profile, Name, &Props, KPTRUE);
}
#endif /* !SP_NO_OBSOLETE */


/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save the profile data to the file descriptor.  Optionally share the 
 *      tag data.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveOutData (
				SpProfile_t	Profile,
				KpFileId	fd,
				KpBool_t	ShareTags)
{
	SpStatus_t		Status;
	SpProfileData_t	FAR *ProfileData;
	KpInt32_t		FilePos;
	KpUInt32_t		FileSize;
	SpTagRecord_t	FAR *TagRecords;
	SpTagRecord_t	FAR *TagRecord;
	SpTagDirEntry_t	FAR *tagArray;
	void 			KPHUGE *tagData;
	KpInt32_t		index;
	KpBool_t		fOk;
	KpInt32_t		activeCount = 0;
	KpUInt32_t		incrementTagRecord = 0;
	KpUInt32_t		ProfID[4];


/* lock down the profile data */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* determine the number of active TagArray elements */
	activeCount = SpTagGetCount (ProfileData);
	
/* allocate space for tag records */
	TagRecord =
	TagRecords = SpMalloc (activeCount * sizeof (*TagRecords));
	if (NULL == TagRecords) {
		SpProfileUnlock (Profile);
		return SpStatMemory;
	}
	
/* zero out TagRecord buffer before using */
	KpMemSet (TagRecord, 0, activeCount * sizeof (*TagRecords));

/* write the Header */
	fOk = KPTRUE;
	SpProfileWriteHeader (&fOk, fd, ProfileData);
	
/* write dummy tag directory */
	SpWriteTagDir (&fOk, fd, activeCount, TagRecords);
	Status = fOk ? SpStatSuccess : SpStatFileWriteError;

/**********************/
/* write the Tag data */
/**********************/

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
		
	if (SpStatSuccess == Status)
	{
	
		for (index = 0; index < ProfileData->TotalCount; index++)
		{
		
		/* determine if this TagArray element holds good data */
			if (tagArray[index].TagDataSize == -1)
				continue;
			else
				incrementTagRecord = 1;
				
		/* determine where this tag will go in the file */
			Status = DoFilePadding (fd, &FilePos);
			if (SpStatSuccess != Status)
				break;

		/* try to share data from previous tags */
			if (!ShareTags || !SpTagShare (tagArray, index,
											TagRecords, TagRecord))
			{

			/* not sharing tag data, write data to file */

			/* remember where we put this tag */
				TagRecord->Id = (KpUInt32_t) tagArray[index].TagId;
				TagRecord->Offset = (KpUInt32_t) FilePos;
				TagRecord->Size = tagArray[index].TagDataSize;
			
			/* write the tag to the file */
				tagData = (void KPHUGE *) lockBuffer (tagArray[index].TagData);
				if (0 == KpFileWrite (fd, tagData, tagArray[index].TagDataSize))
				{
					Status = SpStatFileWriteError;
					unlockBuffer (tagArray[index].TagData);
					break;
				}
				unlockBuffer (tagArray[index].TagData);
			}

		/* point to the next tag directory entry */
			if (incrementTagRecord)
			{
				TagRecord++;
				incrementTagRecord = 0;
			}
		}
	}
	unlockBuffer (ProfileData->TagArray);

	/* Make sure file size is multiple of 4 */
	Status = DoFilePadding (fd, &FilePos);

/*******************************/
/* update the size of the file */
/*******************************/
	if (SpStatSuccess == Status) {
		KpFileTell (fd, &FilePos);
		FileSize = (KpUInt32_t) FilePos;
		KpFilePosition (fd, FROM_START, 0);
		SpWriteUInt32 (&fOk, fd, FileSize);

/* Now update the MD5 Profile ID if it is a new version profile */
		if (ProfileData->Header.ProfileVersion >= SpProfileTagVersion)
		{
			Status = SpProfileGetProfileId(Profile, ProfID);
			KpFilePosition (fd, FROM_START, 84);
			if (0 == KpFileWrite (fd, ProfID, 16))
				Status = SpStatFileWriteError;
		}
	}
	
/********************************/
/* write the real Tag directory */
/********************************/
	if (SpStatSuccess == Status) {
		KpFilePosition (fd, FROM_START, 128);
		SpWriteTagDir (&fOk, fd, activeCount, TagRecords);
		if (!fOk)
			Status = SpStatFileWriteError;
	}

/* done with tag records, free space */
	SpFree (TagRecords);
	SpProfileUnlock (Profile);

/* Now save out the Rendering Intent and Profile Flags,  */
/* zero them out along with the profile ID and then calc */
/* and set the profile ID (md5) and then return the      */
/* Rendering Intent and Profile Flags                    */


	return Status;
}

#if !defined (SP_NO_FILEIO)
/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Save a profile to a named file.  Optionally share the tag data.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveProfileEx (
				SpProfile_t	Profile,
				KpChar_t	*Name,
				SpFileProps_t	*Props,
				KpBool_t	ShareTags)
{
	SpStatus_t		Status;
	SpProfileData_t	FAR *ProfileData;
	KpFileProps_t		props;
	KpFileId		fd;

/* lock down the profile data */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* Validate profile */
	Status = SpProfileValidate (ProfileData);
	SpProfileUnlock (Profile);

	if (SpStatSuccess != Status) {
		return Status;
	}

#if defined (KPMAC)
/* Convert file information into useable form */
	SpCvrtSpFileProps(Props, &props);
#endif

/* delete existing file */
	KpFileDelete (Name, &props);

/* open file */
	if (0 == KpFileOpen (Name, "w", &props, &fd)) {
		return SpStatFileNotFound;
	}

	Status = SpProfileSaveOutData ( Profile, fd, ShareTags);

/* close the file */
	KpFileClose (fd);
	
/* check that all is well */
	if (SpStatSuccess != Status)
		KpFileDelete (Name, &props);

	return Status;
}
#endif /* !SP_NO_FILEIO */



#if !defined (SP_NO_FILEIO)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Save current values for a profile.  Optionally share the tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSaveEx (
				SpProfile_t		Profile,
				KpBool_t			ShareTags)
{
	char			FAR *fileName;
	SpProfileData_t FAR *ProfileData;
	SpStatus_t		Status;

/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* check for case of no file name */
	if (NULL == ProfileData->FileName) {
		SpProfileUnlock (Profile);
		return SpStatNoFileName;
	}

/* lock FileName handle and return ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);
	
/* check for case of no file name */
	if (NULL == fileName) {
		SpProfileUnlock (Profile);
		return SpStatNoFileName;
	}

/* let SpProfileSaveProfileEx do the real work */
	Status = SpProfileSaveProfileEx (Profile, fileName, &ProfileData->Props, ShareTags);

/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Save current values for a profile.  Share the tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSave (
				SpProfile_t		Profile)
{
	return SpProfileSaveEx (Profile, KPTRUE);
}
#endif /* !SP_NO_FILEIO */



     
     
