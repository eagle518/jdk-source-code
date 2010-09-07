/*
 * @(#)sprofpr.c	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the profile functions
			not needed by SUN nor Java Libraries.

			Pulled form sprofile.c on 7/2/99

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2002 by Eastman Kodak Company, 
			all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sprofpr.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/sprofpr.c $
		$Revision: 9 $
		$Date: 3/14/02 2:07p $
		$Author: Msm $

	SCCS Revision:
	@(#)sprofpr.c	1.2	12/19/03

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-1999                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include <string.h>


#if defined(KPMAC)
#include "spcback.h"
#if (!defined KPMACPPC) & (defined KPMW) 
	#include <A4Stuff.h>
#endif


/*--------------------------------------------------------------------
 * DESCRIPTION	(Macintosh Version)
 *	Copies 
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfSetIOFileData (
				SpIOFileChar_t	*src,
				SpIOFileChar_t	*dest)
{
	strncpy (dest->fileType, src->fileType, 5);
	strncpy (dest->creatorType, src->creatorType, 5);
	dest->vRefNum = src->vRefNum;

	return SpStatSuccess;
}
SpStatus_t KSPAPI SpProfSetSpFileData (
				SpFileProps_t	*src,
				SpIOFileChar_t	*dest)
{
	strncpy (dest->fileType, src->fileType, 5);
	strncpy (dest->creatorType, src->creatorType, 5);
	dest->vRefNum = src->vRefNum;

	return SpStatSuccess;
}
/*--------------------------------------------------------------------
 * DESCRIPTION	(Macintosh Version)
 *	Copies 
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfSetSpFileProps (
				SpFileProps_t	*src,
				SpFileProps_t	*dest)
{
	strncpy (dest->fileType, src->fileType, 5);
	strncpy (dest->creatorType, src->creatorType, 5);
	dest->vRefNum = src->vRefNum;
	dest->dirID   = src->dirID;

	return SpStatSuccess;
}
SpStatus_t KSPAPI SpProfSetIOFileProps (
				SpIOFileChar_t	*src,
				SpFileProps_t	*dest)
{
	strncpy (dest->fileType, src->fileType, 5);
	strncpy (dest->creatorType, src->creatorType, 5);
	dest->vRefNum = src->vRefNum;
	dest->dirID   = 0;

	return SpStatSuccess;
}
SpStatus_t KSPAPI SpProfileClearProps (
				SpFileProps_t	*dest)
{
	strncpy (dest->fileType, "     ", 5);
	strncpy (dest->creatorType, "     ", 5);
	dest->vRefNum = 0;
	dest->dirID   = 0;

	return SpStatSuccess;
}

#endif /*KPMAC*/


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete a profile. If there is a file associated with the profile,
 *	it is also deleted.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileDelete (
				SpProfile_t FAR *Profile)
{
	KpChar_t		*fileName;
	SpProfileData_t	FAR *ProfileData;
	KpFileProps_t		Props;

/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (*Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* lock FileName handle and return  ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);

#if defined (KPMAC)
/* Convert SpIOFileChar_t to KpFileProps_t */
	SpCvrtSpFileProps (&(ProfileData->Props), &Props);
#endif

/* delete associated file name */
	if (NULL != fileName)
		KpFileDelete (fileName, &Props);

/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (*Profile);
	return SpProfileFree (Profile);
}



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Returns KPTRUE if seems to be ICC Formatted Profile
 *      private function
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	August 9, 1996
 *------------------------------------------------------------------*/
KpBool_t KSPAPI SpIsICCProfile(	char                    *Filename,
				SpFileProps_t		*Props)
{
KpUInt32_t     ProfileSize;
KpUInt32_t     ProfileCode = 0;
SpStatus_t     Status = SpStatBadProfile;
KpFileId       FD;
char          *name;
KpUInt32_t     Read_Size_OK, Read_Code_OK = 0;
KpInt32_t      Read_Amount = 4;
KpFileProps_t  fileProps;
KpBool_t       IsICC = KPFALSE;
char           ReadBufferArray[8];
char           *ReadBuffer;
 
   name = Filename;
    
    

#if defined (KPMAC)
   SpCvrtSpFileProps(Props, &fileProps);
#endif
   if (KpFileOpen(name, "r", &fileProps, &FD)) /* 0 = not opened */
   {
      /* Read HEADER_SIZE into Buffer */
      ReadBuffer = ReadBufferArray;
      Read_Size_OK =  KpFileRead(FD, ReadBuffer, &Read_Amount); 
      ProfileSize = SpGetUInt32(&ReadBuffer);
    
      if (KpFilePosition(FD, FROM_START, 36))
      {
         ReadBuffer = ReadBufferArray;
         Read_Code_OK = KpFileRead(FD, ReadBuffer, &Read_Amount);
         ProfileCode = SpGetUInt32(&ReadBuffer);
      }

      /* Close File */
      KpFileClose(FD);

      if ((Read_Size_OK) && (ProfileSize > HEADER_SIZE))
      {
         if ((Read_Code_OK) && (ProfileCode == (0x61637370)))
            IsICC = KPTRUE;
      }
   }
   return (IsICC);
} 


#if !defined (SP_NO_FILEIO) || !defined (SP_NO_SEARCH)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Return the header for the profile.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	October 23, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadHeader(
                                char                    *Filename,
				SpFileProps_t		*Props,
                                SpHeader_t              FAR *Header)
{
SpHugeBuffer_t BufferAddress;
SpStatus_t     Status = SpStatBadProfile;
KpFileId       FD;
char          *name;
KpUInt32_t     Read_OK;
KpInt32_t     Read_Amount = HEADER_SIZE;
KpFileProps_t  fileProps;
 
   if (!SpIsICCProfile(Filename, Props))
      return (SpStatBadProfile);

   name = Filename;
    
   BufferAddress = allocBufferPtr(HEADER_SIZE);
   if (BufferAddress == NULL)
      return (SpStatMemory);
    
#if defined (KPMAC)
   SpCvrtSpFileProps(Props, &fileProps);
#endif
   if (KpFileOpen(name, "r", &fileProps, &FD)) /* 0 = not opened */
   {
      /* Read HEADER_SIZE into Buffer */
      Read_OK =  KpFileRead(FD, BufferAddress, &Read_Amount); 
    
      /* Close File */
      if (!KpFileClose(FD))
         Status = SpStatBadProfile; 
    
      if (Read_OK)
      {
         Status = SpHeaderToPublic(BufferAddress, HEADER_SIZE, Header);
      }
   }
   freeBufferPtr(BufferAddress);
   return Status;

}
#endif /* !SP_NO_FILEIO || !SP_NO_SEARCH */

#if !defined (SP_NO_FILEIO)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Return the Tag from the profile.
 *	If the tag is MultiLang return Ascii
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	October 23, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadTag(
                                char                   *Filename,
                                SpFileProps_t          *Props,
                                SpTagId_t               TagId,
                                SpTagValue_t            FAR *Value)
{
SpStatus_t      Status = SpStatSuccess;
SpTagType_t		TagType;
SpUnicodeInfo_t	StringInfo;
KpInt32_t		StringLength;
KpChar_p		Text;
KpInt16_t		Language = 0;
KpInt16_t		Country = 0;

	Status = SpProfileLoadTagEx(Filename, Props,TagId,Value);
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
 *	Return the Tag from the profile.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	March 12, 2002
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadTagEx(
                                char                   *Filename,
                                SpFileProps_t          *Props,
                                SpTagId_t               TagId,
                                SpTagValue_t            FAR *Value)
{
SpHeader_t       Header;
SpHeader_t       *HeaderPtr;
SpHugeBuffer_t   BufferAddress, TagAddress;
SpStatus_t       Status = SpStatSuccess;
KpFileId         FD;
KpFileProps_t    fileProps;
char             name[MAX_PATH];
KpUInt32_t       FilePosition = HEADER_SIZE;
KpInt32_t       Read_Amount  = sizeof(KpInt32_t);
KpInt32_t       i, TagArraySize, TagBufferSize;
SpTagRecord_t   *TagArray; 

   if (!SpIsICCProfile(Filename, Props))
      return (SpStatBadProfile);

   strcpy (name, Filename);
	
   /* Need Header in case we are requesting a Named Color */
   HeaderPtr = (SpHeader_t *)NULL;
   if (TagId == SpTagNamedColor)
   {
      HeaderPtr = &Header;
      Status = SpProfileLoadHeader(name, Props, HeaderPtr);
   }
   if (Status == SpStatSuccess)
   {
      /* Set up for failure - yes I know I'm negative */
      Status = SpStatFileNotFound;

#if defined (KPMAC)
      SpCvrtSpFileProps(Props, &fileProps);
#endif
      if (KpFileOpen(Filename, "r", &fileProps, &FD)) /* 0 = not opened */
      {
         if (KpFilePosition(FD, FROM_START, FilePosition)) 
         {

      	    Status = SpStatFileReadError;
            /* Read TagArraySize into Buffer  */
            if (KpFileRead(FD, &TagArraySize, &Read_Amount)) 
            {
#if defined (KPLSBFIRST)
               /* If we are on a little endian machine we need to do byte swap	*/
               Kp_swab32 (&TagArraySize, 1);
#endif
               TagBufferSize = TagArraySize * 3 * 4;  /* each entry is 3*4 bytes */
               BufferAddress = allocBufferPtr(TagBufferSize);
    	       if (BufferAddress != NULL)
	       {   
                  if (KpFileRead(FD, BufferAddress, &TagBufferSize)) 
                  {
#if defined (KPLSBFIRST)
                  /* If we are on a little endian machine we need to do byte swap	*/
                     Kp_swab32 (BufferAddress, TagBufferSize / sizeof (KpInt32_t));
#endif
                     TagArray = BufferAddress;
      	             Status = SpStatTagNotFound;
                     for (i = 0; i < TagArraySize; i++)
                     {
                        if (TagId == (SpTagId_t)TagArray[i].Id)
                        {
      	                   Status = SpStatMemory;
                           TagAddress = allocBufferPtr(TagArray[i].Size);
                           if (TagAddress != NULL) 
                           {
                              if (KpFilePosition(FD, FROM_START, TagArray[i].Offset))
                              {
                                 Read_Amount = TagArray[i].Size;
                                 if (KpFileRead(FD, TagAddress, &Read_Amount))
                                 {
                                    Status = SpTagToPublic(HeaderPtr,   TagId, 
                                                     TagArray[i].Size,
                                                     TagAddress,  Value);
                                 }
                              }
                              freeBufferPtr(TagAddress);
                           } else
                           {
      	                      Status = SpStatMemory;
                              break;
                           }
                        }
                     }
                  }
                  freeBufferPtr(BufferAddress);
	       } else /* Buffer Address != 0 */
      	          Status = SpStatMemory;
            }
         }
         /* Close File */
         KpFileClose(FD); 
      }
   }
   return Status;

}
#endif /* !SP_NO_FILEIO */



#if !defined (SP_NO_OBSOLETE)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Private -- phase out 5/4/95)
 *	Return the name of the currently associated file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetFileName (
				SpProfile_t		Profile,
				size_t			BufferSize,
				char			FAR *Buffer)
{
	char				FAR *fileName;
	SpProfileData_t		FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* lock FileName handle and return  ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);

/* check for case of no file name */
	if (NULL == fileName) {
		*Buffer = '\0';
		SpProfileUnlock (Profile);
		return SpStatNoFileName;
	}

/* check for buffer large enough */
	if (BufferSize < strlen (fileName) + 1) {
		unlockBuffer (ProfileData->FileName);
		SpProfileUnlock (Profile);
		return SpStatBufferTooSmall;
	}

	strcpy (Buffer, fileName);
	
/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	
	return SpStatSuccess;
}
#endif

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Return the name of the currently associated file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 16, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetName (
				SpProfile_t		Profile,
				size_t			BufferSize,
				KpChar_t		*Buffer,
				SpFileProps_t	*Props)
{
	char				FAR *fileName;
	SpProfileData_t			FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* lock FileName handle and return  ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);

/* check for case of no file name */
	if (NULL == fileName) {
		*Buffer = '\0';
		SpProfileUnlock (Profile);
		return SpStatNoFileName;
	}

/* check for buffer large enough */
	if (BufferSize < strlen (fileName) + 1) {
		unlockBuffer (ProfileData->FileName);
		SpProfileUnlock (Profile);
		return SpStatBufferTooSmall;
	}

	strcpy (Buffer, fileName);

/* Get file properties */
#if defined (KPMAC)
	SpProfSetSpFileProps(&(ProfileData->Props), Props);
#endif
	
/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	
	return SpStatSuccess;
}
#if !defined (SP_NO_OBSOLETE)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Return the name of the currently associated file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	September 20, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetDiskName (
				SpProfile_t		Profile,
				size_t			BufferSize,
				KpChar_t		*Buffer,
				SpIOFileChar_t	*Props)
{
	char				FAR *fileName;
	SpProfileData_t		FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* lock FileName handle and return  ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);

/* check for case of no file name */
	if (NULL == fileName) {
		*Buffer = '\0';
		SpProfileUnlock (Profile);
		return SpStatNoFileName;
	}

/* check for buffer large enough */
	if (BufferSize < strlen (fileName) + 1) {
		unlockBuffer (ProfileData->FileName);
		SpProfileUnlock (Profile);
		return SpStatBufferTooSmall;
	}

	strcpy (Buffer, fileName);

/* Get file properties */
#if defined (KPMAC)
	SpProfSetSpFileData (&(ProfileData->Props), Props);
#endif
	
/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION	(Private -- phase out 5/4/95)
 *	Set the name of the file to associate with the profile.
 *	If the Name is NULL no file will be associated with the profile.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	Octorber 22, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetFileName (
				SpProfile_t		Profile,
				char			FAR *FileName)
{
	char			FAR *fileName;
	SpProfileData_t	FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* Free current FileName if one is there */
	if (ProfileData->FileName != NULL) 
		freeBuffer(ProfileData->FileName);

/* create the FileName handle */
	ProfileData->FileName = allocBufferHandle (strlen (FileName) + 1);
	if (ProfileData->FileName == NULL) 
	   return (SpStatMemory);
	
/* lock FileName handle and return ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);
	if (fileName == NULL) 
	   return (SpStatMemory);
	
/* copy text data into the newly allocated space */
	strcpy (fileName, FileName);
	
#if defined (KPMAC)
	SpProfileClearProps(&(ProfileData->Props));
#endif

/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	return SpStatSuccess;
}
#endif

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Set the name of the file to associate with the profile.
 *	If the Name is NULL no file will be associated with the profile.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE Copied
 *	November 15, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetName (
				SpProfile_t		Profile,
				KpChar_t		*FileName,
				SpFileProps_t	*Props)
{
	char			FAR *fileName;
	SpProfileData_t		FAR *ProfileData;
	SpStatus_t		status;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* Free current FileName if one is there */
	if (ProfileData->FileName != NULL) 
	{
		if (ProfileData->TagArray == NULL)
			status = SpProfilePopTagArray(ProfileData);

		freeBuffer(ProfileData->FileName);
	}

/* create the FileName handle */
	ProfileData->FileName = allocBufferHandle (strlen (FileName) + 1);
	if (ProfileData->FileName == NULL) 
	   return SpStatMemory;
	
/* lock FileName handle and return ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);
	if (fileName == NULL) 
	   return SpStatMemory;
	
/* copy text data into the newly allocated space */
	strcpy (fileName, FileName);

/* Copy props information into internal profile data structure */
#if defined (KPMAC)
	SpProfSetSpFileProps (Props, &(ProfileData->Props));
#endif
	
/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	return SpStatSuccess;
}

#if !defined (SP_NO_OBSOLETE)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Set the name of the file to associate with the profile.
 *	If the Name is NULL no file will be associated with the profile.
 *
 * AUTHOR
 * 	lsh & mlb
 *
 * DATE CREATED
 *	May 4, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetDiskName (
				SpProfile_t		Profile,
				KpChar_t		*FileName,
				SpIOFileChar_t	*Props)
{
	char			FAR *fileName;
	SpProfileData_t	FAR *ProfileData;
	
/* convert profile handle to data pointer */
	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

/* Free current FileName if one is there */
	if (ProfileData->FileName != NULL) 
		freeBuffer(ProfileData->FileName);

/* create the FileName handle */
	ProfileData->FileName = allocBufferHandle (strlen (FileName) + 1);
	if (ProfileData->FileName == NULL) 
	   return SpStatMemory;
	
/* lock FileName handle and return ptr */
	fileName = (char *) lockBuffer (ProfileData->FileName);
	if (fileName == NULL) 
	   return SpStatMemory;
	
/* copy text data into the newly allocated space */
	strcpy (fileName, FileName);

/* Copy props information into internal profile data structure */
#if defined (KPMAC)
	SpProfSetIOFileProps(Props, &(ProfileData->Props));
#endif
	
/* unlock handles */
	unlockBuffer (ProfileData->FileName);
	SpProfileUnlock (Profile);
	return SpStatSuccess;
}


/*--------------------------------------------------------------------
 * DESCRIPTION	(Private -- phase out 5/4/95)
 *	Load a profile from a file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 21, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadFromFile (
				SpCallerId_t	CallerId,
				char			FAR *FileName,
				SpProfile_t		FAR *Profile)
{
	SpFileProps_t	Props;

#if defined (KPMAC)
/* map ths routine to current SpProfileLoadProfile routine */
	Props.vRefNum = Props.dirID = 0;
#endif

/* load the profile */
	return (SpProfileLoadProfile(CallerId, FileName, &Props, Profile));
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Load a profile from a file.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	May 2, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadFromDisk (
				SpCallerId_t	CallerId,
				KpChar_t		*FileName,
				SpIOFileChar_t	*Props,
				SpProfile_t		FAR *Profile)
{
	SpFileProps_t	props;

/* map ths routine to current SpProfileLoadProfile routine */
#if defined (KPMAC)
	SpCvrtIOFileProps(Props, &props);
#endif

/* load the profile */
	return (SpProfileLoadProfile(CallerId, FileName, &props, Profile));

}
#endif

#if !defined (SP_NO_FILEIO)
/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Load a profile from a file.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	December 5, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileLoadProfile (
				SpCallerId_t	CallerId,
				KpChar_t	*FileName,
				SpFileProps_t	*Props,
				SpProfile_t	FAR *Profile)
{
	KpMapFile_t	MapFileCtl;
	KpFileProps_t	props;
	SpStatus_t	Status;
	char		KPHUGE *Ptr;
	KpInt32_t	ProfileSize;


#if defined (KPMAC)
/* Convert file information into useable form */
	SpCvrtSpFileProps(Props, &props);
#endif

/* Map the file */
	if (NULL == KpMapFileEx (FileName, &props, "r", &MapFileCtl))
		return SpStatFileNotFound;

    Ptr = (char *)MapFileCtl.Ptr;
    ProfileSize = (KpInt32_t)SpGetUInt32 (&Ptr);

	if (ProfileSize > MapFileCtl.NumBytes)
	{
		KpUnMapFile (&MapFileCtl);
		return SpStatBadProfile;
	}

/* Load the profile */
	Status = SpProfileLoadFromBuffer (CallerId, MapFileCtl.Ptr, 
                                          Profile);

/* Unmap the file */
	KpUnMapFile (&MapFileCtl);

/* Remember the file name */
	if (SpStatSuccess == Status) {
		Status = SpProfileSetName (*Profile, FileName, Props);
		if (SpStatSuccess != Status)
			SpProfileFree (Profile);
	}

	return Status;
}
#endif /* !SP_NO_FILEIO */

/*--------------------------------------------------------------------
 * DESCRIPTION  (Semi-Private)
 *      Valid the handle points to memory with Sig set correctly
 *
 * AUTHOR
 *      doro
 *
 * DATE CREATED
 *      January 31, 1996
 *------------------------------------------------------------------*/
KpBool_t KSPAPI SpProfileValidHandle(SpProfile_t SpProf)
{
SpProfileData_t		*Pf;
KpBool_t		Result = KPFALSE;
 
   if (SpProf != NULL)
   {

      Pf = SpProfileLock(SpProf);
      if (Pf != NULL)
      {
         if (Pf->Signature == SpProfileDataSig)
            Result = KPTRUE;
         SpProfileUnlock(SpProf);
      }
   }
   return Result;
}


