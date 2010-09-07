/*
 * @(#)sprofile.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the profile functions.

				Created by lsh, September 14, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sprofile.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/sprofile.c $
		$Revision: 6 $
		$Date: 5/07/02 5:36p $
		$Author: Arourke $

	SCCS Revision:
	@(#)sprofile.c	1.94	4/16/99

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

#define SPGLOBAL
#include "sprof-pr.h"
#include <string.h>


#if defined(KPMAC)

#include "spcback.h"
#if (!defined KPMACPPC) & (defined KPMW)
        #include <A4Stuff.h>
#endif
/* Local Prototype */
static SpStatus_t SpDoIter (
			SpTagIter_t	ProgressFunc,
			SpIterState_t	State,
			SpProfile_t	Profile,
			SpTagId_t	TagId,
			void		FAR *Data);

/*--------------------------------------------------------------------
 * DESCRIPTION
 *      Do progress callback. - MAC version
 *
 * AUTHOR
 *      mec
 *
 * DATE CREATED
 *      May 4, 1995
 *------------------------------------------------------------------*/
static SpStatus_t SpDoIter (
			SpTagIter_t	ProgressFunc,
			SpIterState_t	State,
			SpProfile_t	Profile,
			SpTagId_t	TagId,
			void		FAR *Data)
{
#if defined(KPMAC68K)
	volatile long	hostA4, hostA5;
	volatile long	thisA4, thisA5;
#endif
	SpStatus_t	status;
 
	if (NULL == ProgressFunc)
		return SpStatSuccess;
 
#if defined(KPMAC68K)
	/* restore host's global world - we don't know if its an A4 or A5*/
	SPretrieveCallbackA4A5(&hostA4, &hostA5);
	if (hostA5 != 0)
		thisA5 = SetA5(hostA5);
	if (hostA4 != 0)
		thisA4 = SetA4(hostA4);
#endif
 
	status = CallSPIterCallBackFunc((spIterCallBackUPP)ProgressFunc,
					State, Profile, TagId, Data);
 
#if defined (KPMAC68K)
	/* reset our global world */
	if (hostA5 != 0)
		SetA5(thisA5);
	if (hostA4 != 0)
		SetA4(thisA4);
#endif
 
	return status;
}

#else
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Do Tag iter callback. - non MAC version
 *
 * AUTHOR
 * 	mec
 *
 * DATE CREATED
 *	May 5, 1995
 *------------------------------------------------------------------*/
static SpStatus_t SpDoIter (
				SpTagIter_t	ProgressFunc,
				SpIterState_t	State,
				SpProfile_t		Profile,
				SpTagId_t		TagId,
				void			FAR *Data)
{
	if (NULL != ProgressFunc)
		return ProgressFunc (State, Profile, TagId, Data);

	return SpStatSuccess;
}

#endif

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert Profile handle to pointer to locked profile data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpProfileData_t	FAR *SpProfileLock (SpProfile_t Profile)
{
	SpProfileData_t	FAR *ProfileData;

	ProfileData = lockBuffer ((KcmHandle) Profile);
	if (NULL != ProfileData) {
#if defined(KPMAC)
		/* increment the lock counter */
		ProfileData->LockCount++;
#endif
		if (SpProfileDataSig != ProfileData->Signature) {
			unlockBuffer ((KcmHandle) Profile);
			return NULL;
		}
	}
	return ProfileData;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Unlock profile data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
void SpProfileUnlock (SpProfile_t Profile)
{

#if defined(KPMAC)

	SpProfileData_t	FAR *ProfileData;

	if (NULL == Profile)
		return;

	ProfileData = *((SpProfileData_t FAR **) Profile);
	ProfileData->LockCount--;
		
	/* lockCount == 0 means that we need to fully unlock the handle */
	if(ProfileData->LockCount == 0)
#endif
		unlockBuffer ((KcmHandle) Profile);

}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Iterate over all the tags in a profile.  Call user supplied
 *	function with the Id of each tag.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	July 7, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagIter (
				SpProfile_t		Profile,
				SpTagIter_t		TagIterFunc,
				void			FAR *Data)
{
	KpInt32_t		index;
	SpProfileData_t		FAR *ProfileData;
	SpTagDirEntry_t	FAR *tagArray;
	SpStatus_t			Status;

/* convert profile handle to pointer to locked memory */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	Status = SpDoIter ( TagIterFunc, SpIterInit, NULL, 0, Data);

	/* Check if Profile found via Search function */
	if (ProfileData->TagArray == NULL)
		/* If so, it needs the Tag Array Loaded */
		SpProfilePopTagArray(ProfileData);

	for (index = 0; (index < ProfileData->TotalCount) && (Status == SpStatSuccess); index++)
	{
		/* User Callback Function could unlock since the
		   Profile is available, so lock before calling */
		/* lock TagArray handle and return ptr */
		tagArray = (SpTagDirEntry_t FAR *) 
				lockBuffer (ProfileData->TagArray);

		/* call users function ONLY IF VALID ENTRY */	
		if (tagArray[index].TagDataSize != -1)
			Status = SpDoIter(TagIterFunc,
					SpIterProcessing, 
					Profile, 
					tagArray[index].TagId, 
					Data);
	}

	unlockBuffer (ProfileData->TagArray);

	SpDoIter ( TagIterFunc, SpIterTerm, NULL, 0, Data);

	SpProfileUnlock (Profile);
	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Convert profile file to internal format.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 18, 1993
 *------------------------------------------------------------------*/
SpStatus_t SpProfileLoadFromBufferImp (
				SpProfileData_t	FAR *ProfileData,
				char			KPHUGE *BaseAddr)
{
	SpStatus_t		Status;
	char			KPHUGE *Ptr;
	KpUInt32_t		ProfileSize;
	KpUInt32_t		TagDataHeader;
	KpUInt32_t		Offset;
	KpUInt32_t		Count;
	KpUInt32_t		i;
	SpTagId_t		TagId;
	KpUInt32_t		TagOffset;
	KpUInt32_t		TagSize;

/* validate that atleast the first long of the buffer is readable */
	if (BaseAddr == NULL)
		return SpStatBadBuffer;

/* get the long, this is the size of the profile data */
	Ptr = BaseAddr;
	ProfileSize = SpGetUInt32 (&Ptr);

/* validate that the entire buffer is readable */
	if (BaseAddr == NULL)
		return SpStatBadBuffer;

/* convert the header to public form */
	Status = SpHeaderToPublic (BaseAddr, ProfileSize, &ProfileData->Header);
	if (SpStatSuccess != Status)
		return Status;

	TagDataHeader = 128;
	Ptr = BaseAddr + TagDataHeader;

/* check that the offset to the Tag Data is inside the file */
	if (ProfileSize < TagDataHeader + 4)
		return SpStatOutOfRange;

/* get number of tags */
	Offset = TagDataHeader + sizeof (KpUInt32_t);
	if (ProfileSize < Offset)
		return SpStatBadProfile;

	Ptr = BaseAddr + TagDataHeader;
	Count = SpGetUInt32 (&Ptr);

/* validate that tag directory is readable */
	if (ProfileSize < Offset + Count * 3 * sizeof (KpUInt32_t))
		Status = SpStatBadProfileDir;

/* Set the Profile Size and initialize the Profile Changed Flag */
	ProfileData->ProfileSize = ProfileSize;
	ProfileData->ProfChanged = KPFALSE;

/* build tag directory */
	Status = SpStatSuccess;
	for (i = 0; (SpStatSuccess == Status) && (i < Count); i++) {
		TagId = (SpTagId_t) SpGetUInt32 (&Ptr);
		TagOffset = SpGetUInt32 (&Ptr);
		if (ProfileSize < TagOffset)
			return SpStatBadProfileDir;

		/* The following two lines are removed to allow for reading of
			certain ICC Profiles that don't meet the specification.
			This has been a continual support problem and we've decided
			to go with the other CMM vendors and loosen our criteria */
		/* if (TagOffset & 3)
			return SpStatBadProfileDir; */

		TagSize = SpGetUInt32 (&Ptr);
		if (ProfileSize < TagOffset + TagSize)
			return SpStatBadProfileDir;

		Status = SpTagDirEntryAdd (ProfileData, TagId,
					TagSize, BaseAddr + TagOffset);
	}

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create profile that uses supplied buffer as the profile data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadFromBuffer (
				SpCallerId_t	CallerId,
				void			FAR *Buffer,
				SpProfile_t		FAR *Profile)
{
	SpStatus_t		Status;
	SpProfileData_t	FAR *ProfileData;
	
	Status = SpProfileAlloc (CallerId, Profile, &ProfileData);
	if (SpStatSuccess != Status)
		return Status;

/* get profile header into internal format */
	Status = SpProfileLoadFromBufferImp (ProfileData,  Buffer);

	SpProfileUnlock (*Profile);

/* free profile if an error occurred */
	if (SpStatSuccess != Status)
		SpProfileFree (Profile);

	return Status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Return the header for the profile.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	March 18, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetHeader (
				SpProfile_t		Profile,
				SpHeader_t		FAR *Header)
{
	SpProfileData_t FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	*Header = ProfileData->Header;
	SpProfileUnlock (Profile);
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Set the header for the profile.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	March 18, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetHeader (
				SpProfile_t		Profile,
				SpHeader_t		FAR *Header)
{
	SpProfileData_t FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	ProfileData->Header = *Header;
	SpProfileUnlock (Profile);
	return SpStatSuccess;
}


