/*
 * @(#)spsearch.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the profile functions.

				Separated out of sprofile.c
				August 19, 1997

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-2003 by Eastman Kodak Company, 
			all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: spsearch.c $
		$Logfile: /DLL/KodakCMS/sprof_lib/spsearch.c $
		$Revision: 19 $
		$Date: 6/18/03 3:59p $
		$Author: Pcbuild $

	SCCS Revision:
	@(#)spsearch.c	1.30	4/16/99

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1993-2003                 ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "sprof-pr.h"
#include <string.h>

#if defined (KPMAC)
#include <Gestalt.h>
#include <CMApplication.h>
#include <TextUtils.h>
#endif

#if defined (KPUNIX)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#endif

static KpBool_t IsDirectory(KpfileDirEntry_t FAR *fileSearch);

static SpStatus_t SpSearchEngine(	SpCallerId_t	CallerId,
				SpDataBase_p	DataBase,
				SpSearch_t	*SearchCriterion,
				KpBool_t	SubSearch,
				SpProfile_t	*profileList,
                                KpInt32_t	listSize,
                                KpInt32_t	*foundCount,
				KpInt32_t	searchMode,
				KpfileCallBack_t	CBFunc);

static KpBool_t TestFile(KpfileDirEntry_t FAR *fileSearch,
		  SpCallerId_t	CallerId,
		  SpStatus_t	*ReturnStatus,
                  SpSearch_t	*SearchCriterion,
		  SpProfile_t	*profileList,
		  KpInt32_t	listSize,
		  KpInt32_t	TotalAvailable,
		  KpInt32_t	*CurrentIndex);


static KpBool_t IsDirectory(KpfileDirEntry_t FAR *fileSearch)
{
#if defined (KPUNIX)

int		Result;
struct stat	FBuf;
char		FullFile[MAX_PATH];

Result = lstat(fileSearch->fileName, &FBuf);
if (Result == -1)
{
   strcpy(FullFile, fileSearch->osfPrivate.base_Dir);
   strcat(FullFile, "/");
   strcat(FullFile, fileSearch->fileName);
   Result = lstat(FullFile, &FBuf);
}

if ((S_ISREG(FBuf.st_mode)) || (Result == -1))
   return KPFALSE;
else
   return KPTRUE;

#elif defined (KPMAC) || defined (KPMSMAC)

HFileInfo	*fParm = (HFileInfo *)&fileSearch->osfPrivate.parm;

if (fParm->ioFlAttrib & ATTR_DIRECTORY )
   return KPTRUE;
else
   return KPFALSE;

#elif defined (KPWIN32)

if (fileSearch->osfPrivate.pFindData->dwFileAttributes & 
    FILE_ATTRIBUTE_DIRECTORY)
   return KPTRUE;
else
   return KPFALSE;

#else      /* Unknown OS */
   return KPFALSE;
#endif
}



/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Order the list of profiles by newest creation date
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	December 15, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileOrderList(
			SpProfile_t	*profileList,
			KpInt32_t	foundCount)
{
   KpInt32_t	Date, *DateList;
   KpInt32_t	Time, *TimeList;
   SpHeader_t	PfHeader;
   KpHandle_t	DateHandle, TimeHandle;
   int		i, j, z;
   SpStatus_t	Status;
   SpProfile_t	profileToMove;

   if (foundCount < 2)
      return SpStatSuccess;

   if ((DateHandle = allocBufferHandle(foundCount*sizeof(KpInt32_t))) ==
       (KpHandle_t)NULL)
      return SpStatMemory;

   if ((DateList = (KpInt32_t *)lockBuffer(DateHandle)) ==
       (KpInt32_t *)NULL)
   {
      freeBuffer(DateHandle);
      return SpStatBadBuffer;
   }

   if ((TimeHandle = allocBufferHandle(foundCount*sizeof(KpInt32_t))) ==
       (KpHandle_t)NULL)
   {
      freeBuffer(DateHandle);
      return SpStatMemory;
   }
   if ((TimeList = (KpInt32_t *)lockBuffer(TimeHandle)) ==
       (KpInt32_t *)NULL)
   {
      freeBuffer(DateHandle);
      freeBuffer(TimeHandle);
      return SpStatBadBuffer;
   }

   Status = SpProfileGetHeader (profileList[0], &PfHeader);
   Date = 366*PfHeader.DateTime.Year +
           31*PfHeader.DateTime.Month +
              PfHeader.DateTime.Day;
   Time = 3600*PfHeader.DateTime.Hour +
            60*PfHeader.DateTime.Minute +
               PfHeader.DateTime.Second;

   /* Save First Profile's date */
   DateList[0] = Date;
   TimeList[0] = Time;
 
   for (i = 1; i < foundCount; i++)
   {
      /* Get testable version of Time and Date */
      Status = SpProfileGetHeader (profileList[i], &PfHeader);
      Date = 366*PfHeader.DateTime.Year +
              31*PfHeader.DateTime.Month +
                 PfHeader.DateTime.Day;
      Time = 3600*PfHeader.DateTime.Hour +
               60*PfHeader.DateTime.Minute +
                  PfHeader.DateTime.Second;
 
      /* Save in case no move required */
      DateList[i] = Date;
      TimeList[i] = Time;
 
      /* Loop thru those before */
      for (j = 0; j < i; j++)
      {
         /* Test for created before a previous entry */
         if ((Date  > DateList[j]) ||
            ((Date == DateList[j]) && (Time > TimeList[j])))
         {
            /* Save the profile o finterest */
            profileToMove = profileList[i];
            /* Move the older profiles back */
            for (z = i; z > j; z--)
            {
               DateList[z]    = DateList[z-1];
               TimeList[z]    = TimeList[z-1];
               profileList[z] = profileList[z-1];
            }
            /* Put the Profile of interest into its spot */
            DateList[j]    = Date;
            TimeList[j]    = Time;
            profileList[j] = profileToMove;
            /* Check the next one on the complete list (i) */
            break;
         }
      }
   }
 
   freeBuffer(DateHandle);
   freeBuffer(TimeHandle);
   return SpStatSuccess;
}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Test Date Value against Header values.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	October 23, 1995
 *------------------------------------------------------------------*/
KpInt32_t TestHeaderDate(SpDateTime_t *ValA, SpDateTime_t *ValB)
{
   KpInt32_t DateA, DateB;
   KpInt32_t TimeA, TimeB;

   DateA = 366*ValA->Year + 31*ValA->Month + ValA->Day;
   DateB = 366*ValB->Year + 31*ValB->Month + ValB->Day;

   if (DateA < DateB)
      return BEFORE;
   if (DateA > DateB)
      return AFTER;

   TimeA = 3600*ValA->Hour + 60*ValA->Minute + ValA->Second;
   if (TimeA == 0)
      return SAME;

   TimeB = 3600*ValB->Hour;
   if ((ValA->Minute > 0) && (ValA->Second > 0))
      TimeB += 60*ValB->Minute;
   if (ValA->Second > 0)
      TimeB += ValB->Second;

   if (TimeA < TimeB)
      return BEFORE;
   if (TimeA > TimeB)
      return AFTER;

   return SAME;
}
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Test profile with header and or tag search criteria and report status
 *
 * AUTHOR
 * 	Sakey
 *
 * DATE CREATED
 *	July, 2002
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileCheckEx(
                                SpSearch_p              SearchValue,
                                char				*Filename,
								SpFileProps_t		*Props)
{
	SpStatus_t			status;
	SpHeader_t			Header;
	SpTagValue_t		tagValue;

	KpInt32_t			Passed[SPSEARCH_LIST];
	KpInt32_t			Tested[SPSEARCH_LIST];
	KpInt32_t			i, j, critSize;
	SpSearchCriterion_t *criterion;

#define	TAGID_LIST 5	
	SpTagId_t			TagIdsPassed[TAGID_LIST];
	SpTagId_t			TagIdsTested[TAGID_LIST];
	SpTagId_t			*pTagIdsPassed = TagIdsPassed;
	SpTagId_t			*pTagIdsTested = TagIdsTested;
	KpInt32_t			TagCount = 0;

	if (NULL == SearchValue)
		return SpStatIncompatibleArguments;

	critSize = SearchValue->critSize;
	criterion = SearchValue->criterion;

	status = SpProfileLoadHeader (Filename, Props, &Header);

	if (SpStatSuccess != status)
		return status;
	
	
	for (i = 0; i < SPSEARCH_LIST; i++)
	{
		 /* preset all entries in PassedEvalArray to Not Tested and failed . */
		Passed[i]  = 0;
		Tested[i]  = 0;
	}
	
	/* count up the number of tag checks requested. */
	for (i = 0; i < SearchValue->critCount; i++)
	{
		if ((SPSEARCH_ENUMTAGVALUE_EQUAL == criterion->SearchElement) ||
			(SPSEARCH_ENUMTAGVALUE_NOTEQUAL == criterion->SearchElement))
			TagCount++;

		 criterion = (SpSearchCriterion_t *)(critSize + (KpUInt8_p)criterion);
	}
	criterion = SearchValue->criterion;

	if (TAGID_LIST < TagCount)
	{
		/* allocate the tested/passed arrays */
		pTagIdsTested = allocBufferPtr( TagCount * sizeof(SpTagId_t));
		if (NULL == pTagIdsTested)
			return SpStatMemory;

		pTagIdsPassed = allocBufferPtr( TagCount * sizeof(SpTagId_t));
		if (NULL == pTagIdsPassed)
			return SpStatMemory;
	}

	/* preset all entries in Tag PassedEvalArrays to Not Tested and failed . */
	for (i = 0; i < TagCount; i++)
	{
		pTagIdsTested[i] = 0;
		pTagIdsPassed[i] = 0;
	}

   /* Loop thru the parameters, test the value based upon the Field
	*   Set that it Passed if it did.  Set that it was tested always
	*/
	for (i = 0; i < SearchValue->critCount; i++)
	{
		switch (criterion->SearchElement)
		{
		case SPSEARCH_PREFERREDCMM       :
			if (criterion->SearchValue.Signature ==
									(KpInt32_t) Header.CMMType)
			   Passed[SPSEARCH_PREFERREDCMM] = 1;
			Tested[SPSEARCH_PREFERREDCMM]    = 1;
			break;

		case SPSEARCH_VERSION            :
			if (criterion->SearchValue.Value     ==
									 Header.ProfileVersion)
			   Passed[SPSEARCH_VERSION]      = 1;
			Tested[SPSEARCH_VERSION]         = 1;
			break;

		case SPSEARCH_PROFILECLASS       :
			if (criterion->SearchValue.Signature ==
								 Header.DeviceClass)
			   Passed[SPSEARCH_PROFILECLASS] = 1;
			Tested[SPSEARCH_PROFILECLASS]    = 1;
			break;

		case SPSEARCH_DEVICECOLORSPACE   :
			if (criterion->SearchValue.Signature ==
							  Header.DataColorSpace)
			   Passed[SPSEARCH_DEVICECOLORSPACE] = 1;
			Tested[SPSEARCH_DEVICECOLORSPACE]    = 1;
			break;

		case SPSEARCH_CONNECTIONSPACE    :
			if (criterion->SearchValue.Signature ==
					   Header.InterchangeColorSpace)
			   Passed[SPSEARCH_CONNECTIONSPACE] = 1;
			Tested[SPSEARCH_CONNECTIONSPACE]    = 1;
			break;

		 /* Only use ONDATE value for all date testing since more
		  * than one value can be given for an attribute */
		case SPSEARCH_BEFOREDATE         :
			if (BEFORE == TestHeaderDate(
							&Header.DateTime,
							&criterion->SearchValue.Date))
			   Passed[SPSEARCH_ONDATE]          = 1;
			Tested[SPSEARCH_ONDATE]             = 1;
			break;

		case SPSEARCH_ONDATE             :
			if (SAME   == TestHeaderDate(
							&Header.DateTime,
							&criterion->SearchValue.Date))
			   Passed[SPSEARCH_ONDATE]          = 1;
			Tested[SPSEARCH_ONDATE]             = 1;
			break;

		case SPSEARCH_AFTERDATE          :
			if (AFTER  == TestHeaderDate(
							&Header.DateTime,
							&criterion->SearchValue.Date))
			   Passed[SPSEARCH_ONDATE]          = 1;
			Tested[SPSEARCH_ONDATE]             = 1;
			break;

		case SPSEARCH_PLATFORM           :
			if (criterion->SearchValue.Signature ==
									Header.Platform)
			   Passed[SPSEARCH_PLATFORM] = 1;
			Tested[SPSEARCH_PLATFORM]    = 1;
			break;

		case SPSEARCH_PROFILEFLAGS       :
			if (criterion->SearchValue.Value     ==
									   Header.Flags)
			   Passed[SPSEARCH_PROFILEFLAGS] = 1;
			Tested[SPSEARCH_PROFILEFLAGS]    = 1;
			break;

		case SPSEARCH_DEVICEMFG          :
			if (criterion->SearchValue.Signature ==
						  Header.DeviceManufacturer)
			   Passed[SPSEARCH_DEVICEMFG] = 1;
			Tested[SPSEARCH_DEVICEMFG]    = 1;
			break;

		case SPSEARCH_DEVICEMODEL        :
			if (criterion->SearchValue.Signature ==
								 Header.DeviceModel)
			   Passed[SPSEARCH_DEVICEMODEL] = 1;
			Tested[SPSEARCH_DEVICEMODEL]    = 1;
			break;

		case SPSEARCH_DEVICEATTRIBUTESHI :
			if (criterion->SearchValue.Value     ==
						 Header.DeviceAttributes.hi)
			   Passed[SPSEARCH_DEVICEATTRIBUTESHI] = 1;
			Tested[SPSEARCH_DEVICEATTRIBUTESHI]    = 1;
			break;

		case SPSEARCH_DEVICEATTRIBUTESLO :
			if (criterion->SearchValue.Value     ==
						 Header.DeviceAttributes.lo)
			   Passed[SPSEARCH_DEVICEATTRIBUTESLO] = 1;
			Tested[SPSEARCH_DEVICEATTRIBUTESLO]    = 1;
			break;

		case SPSEARCH_RENDERINGINTENT    :
			if (criterion->SearchValue.Signature ==
							 (KpInt32_t) Header.RenderingIntent)
			   Passed[SPSEARCH_RENDERINGINTENT] = 1;
			Tested[SPSEARCH_RENDERINGINTENT]    = 1;
			break;

		case SPSEARCH_ILLUMINANT         :
			if ((criterion->SearchValue.XYZ.X    ==
								Header.Illuminant.X) &&
				(criterion->SearchValue.XYZ.Y    ==
								Header.Illuminant.Y) &&
				(criterion->SearchValue.XYZ.Z    ==
								Header.Illuminant.Z))
			   Passed[SPSEARCH_ILLUMINANT] = 1;
			Tested[SPSEARCH_ILLUMINANT]    = 1;
			break;

		case SPSEARCH_ORIGINATOR         :
		   if (criterion->SearchValue.Signature ==
								Header.Originator)
			   Passed[SPSEARCH_ORIGINATOR] = 1;
		   Tested[SPSEARCH_ORIGINATOR]    = 1;
		   break;

		case SPSEARCH_PROFILEID         :
		   if ((criterion->SearchValue.ProfileID.a ==
								Header.ProfileID.a) &&
			   (criterion->SearchValue.ProfileID.b ==
								Header.ProfileID.b) &&
			   (criterion->SearchValue.ProfileID.c ==
								Header.ProfileID.c) &&
			   (criterion->SearchValue.ProfileID.d ==
								Header.ProfileID.d))
			   Passed[SPSEARCH_ORIGINATOR] = 1;
		   Tested[SPSEARCH_ORIGINATOR]    = 1;
		   break;

		case SPSEARCH_ENUMTAGVALUE_EQUAL	:
		case SPSEARCH_ENUMTAGVALUE_NOTEQUAL	:		
			/* find the matching tag ID or the first unused one */
			for(j = 0; j < TagCount; j++)
			{
				if ((pTagIdsTested[j] == criterion->SearchValue.Tag.TagId) || 
					(0 == pTagIdsTested[j]))
				{
					/* mark this tag as tested */
					pTagIdsTested[j] = criterion->SearchValue.Tag.TagId;
					break;
				}
			}
			/* load the tag */
			status = SpProfileLoadTag(Filename, Props, criterion->SearchValue.Tag.TagId, &tagValue);
			if (SPSEARCH_ENUMTAGVALUE_EQUAL == criterion->SearchElement) 
			{
				if ((SpStatSuccess == status) &&
					(criterion->SearchValue.Tag.TagEnum == tagValue.Data.TagEnum))
				{
					pTagIdsPassed[j] = pTagIdsTested[j];
				}
			}
			else
			{
				if ((SpStatSuccess != status) ||
					(criterion->SearchValue.Tag.TagEnum != tagValue.Data.TagEnum))
				{
					pTagIdsPassed[j] = pTagIdsTested[j];
				}
			}
			if (SpStatSuccess == status)
			{
				/* free allocated resources */
				SpTagFree(&tagValue);
			}
			break;
		case SPSEARCH_TIMEORDER:
		default:
		   /* No special processing */
		   break;
		}
 		criterion = (SpSearchCriterion_t *)(critSize + (KpUInt8_p)criterion);
	}
	/* assume success */ 
	status = SpStatSuccess;
   /* Loop thru enumeration types, Checking Passed vs Tested
	* for return value
	*/
	for (i = 0; i < SPSEARCH_LIST; i++)
	{
		if (Tested[i] && !Passed[i])
			status = SpStatBadProfile;
	}

	if (0 < TagCount)
	{
		for(j = 0; j < TagCount; j++)
		{
			if (pTagIdsTested[j] != pTagIdsPassed[j])
				status = SpStatBadProfile;
		}

		if (TAGID_LIST < TagCount)
		{
			freeBufferPtr( (KpGenericPtr_t)pTagIdsTested);
			freeBufferPtr( (KpGenericPtr_t)pTagIdsPassed);
		}
	}
	return status;

}

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Test profile header with search criteria and report status
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	October 23, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileCheck(
                                SpSearch_p              SearchValue,
                                SpHeader_t              FAR *Header)
{
KpInt32_t			Passed[SPSEARCH_LIST];
KpInt32_t			Tested[SPSEARCH_LIST];
KpInt32_t			i, critSize;
SpSearchCriterion_t *criterion;

if (SearchValue != (SpSearch_t *)NULL)
{
  critSize = SearchValue->critSize;
  criterion = SearchValue->criterion;
  for (i = 0; i < SPSEARCH_LIST; i++)
   {
      /* preset all entries in PassedEvalArray to Not Tested and failed . */
      Passed[i]  = 0;
      Tested[i]  = 0;
   }
   
   /* Loop thru the parameters, test the value based upon the Field
    *   Set that it Passed if it did.  Set that it was tested always
    */
   for (i = 0; i < SearchValue->critCount; i++)
   {
      switch (criterion->SearchElement) {
         case SPSEARCH_PREFERREDCMM       :
            if (criterion->SearchValue.Signature ==
                                    (KpInt32_t) Header->CMMType)
               Passed[SPSEARCH_PREFERREDCMM] = 1;
            Tested[SPSEARCH_PREFERREDCMM]    = 1;
            break;
    
         case SPSEARCH_VERSION            :
            if (criterion->SearchValue.Value     ==
                                     Header->ProfileVersion)
               Passed[SPSEARCH_VERSION]      = 1;
            Tested[SPSEARCH_VERSION]         = 1;
            break;
    
         case SPSEARCH_PROFILECLASS       :
            if (criterion->SearchValue.Signature ==
                                 Header->DeviceClass)
               Passed[SPSEARCH_PROFILECLASS] = 1;
            Tested[SPSEARCH_PROFILECLASS]    = 1;
            break;
    
         case SPSEARCH_DEVICECOLORSPACE   :
            if (criterion->SearchValue.Signature ==
                              Header->DataColorSpace)
               Passed[SPSEARCH_DEVICECOLORSPACE] = 1;
            Tested[SPSEARCH_DEVICECOLORSPACE]    = 1;
            break;
    
         case SPSEARCH_CONNECTIONSPACE    :
            if (criterion->SearchValue.Signature ==
                       Header->InterchangeColorSpace)
               Passed[SPSEARCH_CONNECTIONSPACE] = 1;
            Tested[SPSEARCH_CONNECTIONSPACE]    = 1;
            break;
    
         /* Only use ONDATE value for all date testing since more
          * than one value can be given for an attribute */
         case SPSEARCH_BEFOREDATE         :
            if (BEFORE == TestHeaderDate(
                            &Header->DateTime,
                            &criterion->SearchValue.Date))
               Passed[SPSEARCH_ONDATE]          = 1;
            Tested[SPSEARCH_ONDATE]             = 1;
            break;
    
         case SPSEARCH_ONDATE             :
            if (SAME   == TestHeaderDate(
                            &Header->DateTime,
                            &criterion->SearchValue.Date))
               Passed[SPSEARCH_ONDATE]          = 1;
            Tested[SPSEARCH_ONDATE]             = 1;
            break;
    
         case SPSEARCH_AFTERDATE          :
            if (AFTER  == TestHeaderDate(
                            &Header->DateTime,
                            &criterion->SearchValue.Date))
               Passed[SPSEARCH_ONDATE]          = 1;
            Tested[SPSEARCH_ONDATE]             = 1;
            break;
    
         case SPSEARCH_PLATFORM           :
            if (criterion->SearchValue.Signature ==
                                    Header->Platform)
               Passed[SPSEARCH_PLATFORM] = 1;
            Tested[SPSEARCH_PLATFORM]    = 1;
            break;
   
         case SPSEARCH_PROFILEFLAGS       :
            if (criterion->SearchValue.Value     ==
                                       Header->Flags)
               Passed[SPSEARCH_PROFILEFLAGS] = 1;
            Tested[SPSEARCH_PROFILEFLAGS]    = 1;
            break;
    
         case SPSEARCH_DEVICEMFG          :
            if (criterion->SearchValue.Signature ==
                          Header->DeviceManufacturer)
               Passed[SPSEARCH_DEVICEMFG] = 1;
            Tested[SPSEARCH_DEVICEMFG]    = 1;
            break;
    
         case SPSEARCH_DEVICEMODEL        :
            if (criterion->SearchValue.Signature ==
                                 Header->DeviceModel)
               Passed[SPSEARCH_DEVICEMODEL] = 1;
            Tested[SPSEARCH_DEVICEMODEL]    = 1;
            break;
    
         case SPSEARCH_DEVICEATTRIBUTESHI :
            if (criterion->SearchValue.Value     ==
                         Header->DeviceAttributes.hi)
               Passed[SPSEARCH_DEVICEATTRIBUTESHI] = 1;
            Tested[SPSEARCH_DEVICEATTRIBUTESHI]    = 1;
            break;
    
         case SPSEARCH_DEVICEATTRIBUTESLO :
            if (criterion->SearchValue.Value     ==
                         Header->DeviceAttributes.lo)
               Passed[SPSEARCH_DEVICEATTRIBUTESLO] = 1;
            Tested[SPSEARCH_DEVICEATTRIBUTESLO]    = 1;
            break;
    
         case SPSEARCH_RENDERINGINTENT    :
            if (criterion->SearchValue.Signature ==
                             (KpInt32_t) Header->RenderingIntent)
               Passed[SPSEARCH_RENDERINGINTENT] = 1;
            Tested[SPSEARCH_RENDERINGINTENT]    = 1;
            break;
    
         case SPSEARCH_ILLUMINANT         :
            if ((criterion->SearchValue.XYZ.X    ==
                                Header->Illuminant.X)
            &&  (criterion->SearchValue.XYZ.Y    ==
                                Header->Illuminant.Y)
            &&  (criterion->SearchValue.XYZ.Z    ==
                                Header->Illuminant.Z))
               Passed[SPSEARCH_ILLUMINANT] = 1;
            Tested[SPSEARCH_ILLUMINANT]    = 1;
            break;

		case SPSEARCH_ORIGINATOR         :
           if (criterion->SearchValue.Signature ==
                                Header->Originator)
               Passed[SPSEARCH_ORIGINATOR] = 1;
           Tested[SPSEARCH_ORIGINATOR]    = 1;
           break;

		case SPSEARCH_PROFILEID         :
           if ((criterion->SearchValue.ProfileID.a ==
                                Header->ProfileID.a) &&
               (criterion->SearchValue.ProfileID.b ==
                                Header->ProfileID.b) &&
               (criterion->SearchValue.ProfileID.c ==
                                Header->ProfileID.c) &&
               (criterion->SearchValue.ProfileID.d ==
                                Header->ProfileID.d))
               Passed[SPSEARCH_ORIGINATOR] = 1;
           Tested[SPSEARCH_ORIGINATOR]    = 1;
           break;


		case SPSEARCH_ENUMTAGVALUE_EQUAL	:
		case SPSEARCH_ENUMTAGVALUE_NOTEQUAL	:
			return SpStatIncompatibleArguments;

        case SPSEARCH_TIMEORDER:
        default:
           /* No special processing */
		   break;
      }
 	  criterion = (SpSearchCriterion_t *)(critSize + (KpUInt8_p)criterion);
  }
   
   /* Loop thru enumeration types, Checking Passed vs Tested
    * for return value
    */
   for (i = 0; i < SPSEARCH_LIST; i++)
   {
      if (Tested[i] && !Passed[i])
         return SpStatBadProfile;
   }
}
return SpStatSuccess;

}



#if defined (KPWIN32)

extern	HANDLE	SpHInst;
#define SpColorStr			12900	/* Color */
#define DEFPROFILEDBSIZE	260		/* defProfileDB size */

#if defined(_UNICODE)
#define GETCOLORDIRNAME	"GetColorDirectoryW"
#else
#define	GETCOLORDIRNAME "GetColorDirectoryA"
#endif

/*	Constant stings used to find the default profile 
 *	data base path in the registry.
 */
static const char prefKPCMS []       = "kpcms.ini";
static const char sectICCProfiles [] = "IniFileMapping\\kpcms.ini\\ICC Profiles";
static const char keyDefPath []      = "Default Path";
static const char win95ProfileDB []  = "\\Color";

static	BOOL(FAR PASCAL *GetColorDirFunc)(LPCTSTR pMachiName, PTSTR pBuffer, PDWORD pdwSize);

static
BOOL GetFunctionAddress (HINSTANCE hInst, LPCSTR Name, FARPROC FAR *Func)
{
	*Func = NULL;
	*Func = GetProcAddress (hInst, Name);
	if (*Func)
		return TRUE;
	else
		return FALSE;
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(private)
 *	Get the default profile data base path for windows.  The path
 *	is returned in the buffer pointed to by pDataBasePath.  The 
 *	size of the buffer in bytes is specified by size.
 *
 * AUTHOR
 * 	M. Biamonte
 *
 * DATE CREATED
 *	November 16, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI GetWinDefaultDB (KpTChar_p pDataBasePath, KpInt32_t size)
{

	KpInt32_t			retVal;
	KpChar_t			*p;
	HANDLE				hFindFile;
	WIN32_FIND_DATA		findData;
	char 				defProfileDB [DEFPROFILEDBSIZE];
	DWORD				dwSize = DEFPROFILEDBSIZE;
	KpChar_t			FAR ColorDir[256];
	HMODULE				hLib;
	BOOL				result = FALSE;

	*pDataBasePath = '\0';

	hLib = LoadLibrary("mscms.dll");
	if (hLib != NULL)
	{
		result = GetFunctionAddress(hLib, GETCOLORDIRNAME, (FARPROC FAR *)&GetColorDirFunc);
		if (result == TRUE) 
		{
			result = GetColorDirFunc(NULL, defProfileDB, &dwSize);
		}
		FreeLibrary(hLib);
	}
	if(result == FALSE)
	{
		/* Create the {Windows System Directory}\Color
		   directory to use as the default */
		retVal = GetSystemDirectory (defProfileDB, sizeof (defProfileDB));
		if (0 == retVal) {
			return SpStatFileNotFound;
		}

		/* Check for room to add  \\Color */
		LoadString(SpHInst, SpColorStr, (LPSTR)ColorDir, 255);
		if (retVal >= (KpInt32_t) (size - strlen (ColorDir) + 1)) {
			return SpStatBufferTooSmall;
		} 
		strcat (defProfileDB, KcpFileDirSep);
		strcat (defProfileDB, ColorDir);
	}

	/* Read default DB from registry.  If one is not present, set registry to
	   default of windows\system\color.  Note - this doesn't use the
	   KpReadStringPreferenceEx call so that the ini file stuff is skipped.  */

	retVal = KpReadRegistryString ( KPLOCAL_MACHINE, 
			(KpChar_p) sectICCProfiles,
			(KpChar_p) keyDefPath, 
			pDataBasePath, 
			(KpUInt32_p)&size);

	if (KCMS_SUCCESS != retVal) {
		if (KCMS_MORE_DATA == retVal) {
			return SpStatBufferTooSmall;
		}
		else {
			/* write registry entry */
			retVal = KpWriteRegistryString (KPLOCAL_MACHINE, (KpChar_p) sectICCProfiles,
							(KpChar_p) keyDefPath, defProfileDB);
			if (KCMS_SUCCESS != retVal)
				return SpStatFileNotFound;
			strncpy (pDataBasePath, defProfileDB, size);
		}
	}

	/* if trailing file separator - remove it */
	p = strrchr (pDataBasePath, '\\');
	if (p != NULL) {
		if (*(p+1) == '\0') {
			*p = '\0';
		}
	}

	/*	Check if path exists	*/
	hFindFile = FindFirstFile (pDataBasePath, &findData);

	if (INVALID_HANDLE_VALUE == hFindFile) {
		FindClose(hFindFile);
		return (SpStatFileNotFound);
	}
	FindClose(hFindFile);
	if (0 == (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {		
		return (SpStatFileNotFound);
	}

	return SpStatSuccess;

}

#endif /* if defined KPWIN32 */

/*--------------------------------------------------------------------
 * Routine to return the number of default databases for searching profiles
 *   on a platform 
 *
 * AUTHOR
 * 	Anne Rourke
 *
 * DATE CREATED
 *	April 24, 1997
 *------------------------------------------------------------------*/
static KpInt32_t GetDefaultDBCount(void)
{
#if defined (KPSUN)
  return 2;
#elif defined (KPMAC)
/* Check if we're on OS X */
long	macOSVersion;
	if (Gestalt(gestaltSystemVersion, &macOSVersion) == noErr)
	{
		if (macOSVersion >= 0x1000) 
			return 3;
	}
	return 1;

#else
  return 1;
#endif
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Return the default directory name(s) where the profiles
 *	were loaded.  The argument Entries points to the first element in
 *	an array of SpDataBaseEntry structures.  The number of entries in
 *	the array is specified in the location pointed to by numEntries.
 *	The size of the dirName field in each entry in the array is specified 
 *	in the location pointed to by entrySize.  If the dirName field is not
 *	large enough to hold the default directory names the size required 
 *	is returned in the location pointed to by entrySize. 
 *
 * AUTHOR
 * 	Marleen_Clark & doro & M. Biamonte
 *
 * DATE CREATED
 *	October 16, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileGetDefaultDB(
				KpInt32_t		numEntries,
				KpInt32_t		FileNameSize,
				SpDataBaseEntry_p	Entries)
{
KpInt32_t	numDBs;
SpStatus_t 	spStat = SpStatSuccess;
KpInt32_t	count;
#if defined (KPSGI) || defined(KPSGIALL) || defined (KPSUN)
KpBool_t	DoesExist;
dirStatus	Dstat;
#endif

#if defined (KPMAC)
long		gestaltResponse;
OSErr       theErr;
long        theDirID;
short       theVRefNum;
char		fname[MAX_PATH];
CInfoPBRec	parm; /* parameters for call to ROM */
HFileInfo	*fParm = (HFileInfo *)&parm;
DirInfo		*dParm = (DirInfo *)&parm;
Boolean		async=false, ColorSyncAvailable=false;
short       folderID[3] = {kLocalDomain, kSystemDomain, kUserDomain};
KpInt32_t	i;
#endif
long	macOSVersion = 0;

#if defined (KPWIN32)
KpInt32_t		retVal;
#endif

	numDBs = GetDefaultDBCount();

#if defined (KPMAC)
	Gestalt(gestaltSystemVersion, &macOSVersion);
#endif

	if (macOSVersion >= 0x1000) {	/* Only will be non-zero on Mac */
		if (numEntries <=0)
      		return SpStatBufferTooSmall;
      	count = numDBs;	/* keep number of DBs we know about */
    }
    else {
      	if (numEntries < numDBs)
     		return SpStatBufferTooSmall;
    }

   numDBs = 0;

#if defined (KPSUN)
   if (FileNameSize < 40)  /* Testing against the longer 
                                   of the following two 
                                   directory paths */
      return SpStatBufferTooSmall;

   strcpy(Entries[numDBs].dirName, 
         "/usr/openwin/share/etc/devdata/profiles");
   Dstat = KpFileExists(Entries[numDBs].dirName,
                        &Entries[numDBs].fileProps,
                        &DoesExist);
   if (DoesExist == KPTRUE)
      numDBs++;

   strcpy(Entries[numDBs].dirName,
         "/etc/openwin/devdata/profiles");
   Dstat = KpFileExists(Entries[numDBs].dirName,
                        &Entries[numDBs].fileProps,
                        &DoesExist);
   if (DoesExist == KPTRUE)
      numDBs++;
      
     /* Set filenames of unused DBs to NULL */
   while (numDBs < numEntries)
   	 Entries[numDBs++].dirName[0] = '\0';

#else

#if defined (KPSGI) || defined(KPSGIALL)

   if (FileNameSize < 18)   /* Testing against the string size */
      return SpStatBufferTooSmall;

   strcpy(Entries[0].dirName,
         "/var/cms/profiles");

   Dstat = KpFileExists(Entries[numDBs].dirName, 
                        &Entries[numDBs].fileProps,
                        &DoesExist);
   if (DoesExist == KPTRUE)
      numDBs++;

     /* Set filenames of unused DBs to NULL */
   while (numDBs < numEntries)
   	 Entries[numDBs++].dirName[0] = '\0';

#elif defined (KPMAC)

#if defined (KPMACPPC)
	ColorSyncAvailable =
		((Gestalt(gestaltColorMatchingAttr, &gestaltResponse) == noErr)
			&& (gestaltResponse & (1<<gestaltColorMatchingLibLoaded)));
#else defined (KPMAC68K)
	if (Gestalt(gestaltColorMatchingVersion, &version) == noErr) {
		if (version >= gestaltColorSync20) {
			ColorSyncAvailable = true;
		}
	}
#endif

	if (macOSVersion < 0x1000)
	{
		if (ColorSyncAvailable == true) {
			theErr = CMGetColorSyncFolderSpec ( kOnSystemDisk, kDontCreateFolder,
   										&theVRefNum, &theDirID);
   		} else {
   			theErr = colorSyncNotInstalled;
   		}

		if (theErr == noErr) {
			strcpy(Entries[0].dirName, "");
			Entries[0].fileProps.dirID   = theDirID;
			Entries[0].fileProps.vRefNum = theVRefNum;
			numDBs++;
		} else {
			theErr = FindFolder(kOnSystemDisk, kPreferencesFolderType,
                       				kDontCreateFolder, &theVRefNum, &theDirID);


			if (theErr == noErr) {
				/* Get the folder name */
				GetIndString((StringPtr)fname, COLORSYNCSTR, 1);
				if (fname[0] == 0) { /* test for empty pascal string */
					return SpStatFileNotFound;
				}
				if (fname[0] > (unsigned long)FileNameSize) {
					return SpStatBufferTooSmall;
				}
				fParm->ioCompletion = nil;
				fParm->ioVRefNum    = theVRefNum;
				fParm->ioDirID      = theDirID;
				fParm->ioFDirIndex  = 0;
				fParm->ioNamePtr    = (StringPtr)fname;
				theErr = PBGetCatInfo(&parm,async);

				if (theErr == noErr) {
					strcpy(Entries[0].dirName, "");
					Entries[0].fileProps.dirID   = dParm->ioDrDirID;
					Entries[0].fileProps.vRefNum = fParm->ioVRefNum;
					numDBs++;
				} else {
					return SpStatFileNotFound;
				}
			} else {
				return SpStatFileNotFound;
			}
		}
	}
	else
	{	/* OS X Implementation */
		/* First find profiles folder */
			for (i = 0; (i < numEntries) && (i < count); i++)
			{
			
				theErr = FindFolder(folderID[i], kColorSyncProfilesFolderType,
                       				kCreateFolder, &theVRefNum, &theDirID);
				if (theErr == noErr) {
					strcpy(Entries[i].dirName, "");
					Entries[i].fileProps.dirID   = theDirID;
					Entries[i].fileProps.vRefNum = theVRefNum;
					numDBs++;
				} else {
					spStat =  SpStatFileNotFound;
				}
            }
    }
   
     /* Set filenames and Props of unused DBs to NULL */
	while (numDBs < numEntries) {
		Entries[numDBs].dirName[0] = '\0';
		Entries[numDBs].fileProps.dirID   = 0;
		Entries[numDBs++].fileProps.vRefNum = 0;   	
	} 

#elif defined (KPWIN32)

	retVal = GetWinDefaultDB (Entries[0].dirName, FileNameSize);
	if (retVal == SpStatSuccess) {
		numDBs++;
	}
	else {
		return retVal;
	}
	
     /* Set filenames of unused DBs to NULL */
   while (numDBs < numEntries)
   	 Entries[numDBs++].dirName[0] = '\0';
#else

   return SpStatFileNotFound;

#endif

#endif
   return spStat;
}

static KpBool_t TestFile(KpfileDirEntry_t FAR *fileSearch,
		  SpCallerId_t	CallerId,
		  SpStatus_t	*ReturnStatus,
                  SpSearch_t	*SearchCriterion,
		  SpProfile_t	*profileList,
		  KpInt32_t	listSize,
		  KpInt32_t	TotalAvailable,
		  KpInt32_t	*CurrentIndex)
{

SpProfile_t		*profPtr;
SpHeader_t		Header;
SpStatus_t		Status;
SpFileProps_t	FileProp;
SpProfileData_t FAR	*ProfData;
KpBool_t		returnBool = KPTRUE;	/* assume we will continue */
char			*filename;
KpInt32_t		searchMode = SEARCH_RECURSIVE | SPSEARCHFORALL;
SpDataBase_t		DataBase;
char			Dir1[MAX_PATH];
SpDataBaseEntry_t	List[1];

#if defined (KPMAC)
KpUInt16_t		vRefNumSave;
KpUInt32_t		dirIDSave;
int			i;
#endif

if ((fileSearch->opStatus == IOFILE_FINISHED) ||
    (fileSearch->opStatus == IOFILE_STARTED) ||
	(fileSearch->opStatus == IOFILE_ERROR))
{
   return returnBool;
}

#if defined (KPMAC)
   FileProp.vRefNum = fileSearch->osfPrivate.parm.dirInfo.ioVRefNum;
   FileProp.dirID   = fileSearch->osfPrivate.parm.dirInfo.ioDrParID;
#endif

   /* Search into sub-directories */
   if (IsDirectory(fileSearch))
   {
      DataBase.numEntries = 1;
      if (fileSearch->nwAttr & ATTR_DIRECTORY)
      	searchMode |= SEARCH_NOT_RECURSIVE;
      if (fileSearch->nwAttr & ATTR_LINK)
      	searchMode |= SEARCH_NOT_RESOLVE_ALIAS;
      	
#if defined (KPMAC)
	// Setup vRefNum/DirID of subdirectory.  Save original and restore vefore returning
  	 List[0].fileProps.vRefNum = fileSearch->osfPrivate.parm.dirInfo.ioVRefNum;
   	List[0].fileProps.dirID   = fileSearch->osfPrivate.parm.dirInfo.ioDrParID;
	vRefNumSave = fileSearch->osfPrivate.vRefNum;
	dirIDSave = fileSearch->osfPrivate.dirID;
	
	// Only search one level of subdirectory on Mac
	searchMode |= SEARCH_NOT_RECURSIVE;
#endif
#if defined (KPUNIX)
      strcpy(Dir1, fileSearch->osfPrivate.base_Dir);
      strcat(Dir1, "/");
      strcat(Dir1, fileSearch->fileName);
#else
      strcpy(Dir1, fileSearch->fileName);
#endif

      List[0].dirName  = (char *)Dir1;
      DataBase.Entries = List;
      Status = SpSearchEngine(CallerId, &DataBase, SearchCriterion, KPFALSE,
                    profileList, listSize, CurrentIndex, searchMode,
                    (KpfileCallBack_t)TestFileCB);
#if defined (KPMAC)
	 fileSearch->osfPrivate.vRefNum = vRefNumSave;
	 fileSearch->osfPrivate.dirID = dirIDSave;
#endif
      if (Status == SpStatSuccess)
         return KPTRUE;
      else
         return KPFALSE;
   }

   Status = SpProfileLoadHeader(fileSearch->fileName,
                                    &FileProp, &Header);
   if (Status == SpStatSuccess)
   {
      Status = SpProfileCheck(SearchCriterion, &Header);
      if (Status == SpStatSuccess)
      {
         if (*CurrentIndex < listSize)
         {
            profPtr = &profileList[*CurrentIndex];

            Status = SpProfileAlloc(CallerId,
                                    profPtr,
                                    &ProfData);
            if (Status == SpStatSuccess)
            {
               if ((Status = SpProfileSetHeader(*profPtr,
                                                &Header)) ==
                    SpStatSuccess)
               {
                  ProfData->FileName = allocBufferHandle(
                     strlen(fileSearch->fileName) + 1);
                  if (ProfData->FileName == 0)
                  {
                     SpProfileFree(profPtr);
                     return KPFALSE;
                  }
                  filename = (char *)lockBuffer(ProfData->FileName);
                  if (filename == 0)
                  {
                     freeBuffer(ProfData->FileName);
                     SpProfileFree(profPtr);
                     return KPFALSE;
                  }
                  strcpy(filename, fileSearch->fileName);
	              unlockBuffer (ProfData->FileName);
                  ProfData->TotalCount = 0;
                  freeBuffer(ProfData->TagArray);
                  ProfData->TagArray   = NULL;
#if defined (KPMAC)
                  ProfData->Props.vRefNum     = fileSearch->osfPrivate.vRefNum;
                  ProfData->Props.dirID       = fileSearch->osfPrivate.dirID;
                  for (i= 0; i < 5; i++)
                  {
                     ProfData->Props.fileType[i]    = NULL;
                     ProfData->Props.creatorType[i] = NULL;
                  }
#endif

                  (*CurrentIndex)++;
                  if ((*CurrentIndex >= listSize) &&
                      (TotalAvailable == 0))
                     returnBool = KPFALSE;
               } else 
               {
                  *ReturnStatus = Status;
                  returnBool = KPFALSE;
               } /* if SpProfileSetHeader */
               SpProfileUnlock(*profPtr);
            } else
            {
               *ReturnStatus = (KpInt32_t)Status;
               returnBool = KPFALSE;
            } /* if SpProfileAlloc */
         } else
            (*CurrentIndex)++;
      } /* if SpProfileCheck */
   } /* if SpProfileLoadHeader */
   return returnBool;
}


static KpBool_t TestFileCB(KpfileDirEntry_t FAR *fileSearch,
                           Limits_t                 *Limits)
{
SpCallerId_t		CallerId = Limits->CallerId;
SpSearch_t		*SearchCriterion = Limits->SearchCriterion;
SpProfile_t		*profileList = Limits->profileList;
KpInt32_t		listSize = Limits->listSize;
KpInt32_t		*CurrentIndex = &(Limits->CurrentIndex);
KpBool_t		Result;
SpStatus_t		*RetStatus = &(Limits->RetStatus);
KpInt32_t		GetALL = Limits->GetALL;

/* This call back is for a search 
   of profiles in the directory.   */

Result = TestFile(fileSearch,
		  CallerId, RetStatus, SearchCriterion, profileList,
		  listSize, GetALL, CurrentIndex);
return Result;
}

static SpStatus_t SpSearchEngine(SpCallerId_t	CallerId,
				SpDataBase_p	DataBase,
				SpSearch_t	*SearchCriterion,
				KpBool_t	SubSearch,
				SpProfile_t	*profileList,
                                KpInt32_t	listSize,
                                KpInt32_t	*foundCount,
				KpInt32_t	searchMode,
				KpfileCallBack_t	CBFunc)
{
SpStatus_t		status = SpStatSuccess;
KpBool_t		Fstatus;
KpHandle_t		Handle;
Limits_t		*Limits;
KpInt32_t		ProfileCount = listSize;
KpInt32_t		i, critSize;
KpfileDirEntry_t	fileSearch;
KpInt32_t		FindAll = (searchMode & SPSEARCHFORALL);
SpSearchCriterion_t *criterion;

   /* Allocate space to point to the search criteria,
    * point to the SpProfile array, give the max profiles 
    * to find, and give the total found so far
    */
   if ((Handle = allocBufferHandle(sizeof(Limits_t))) !=
       (KpHandle_t)NULL)
   {
      if ((Limits = (Limits_t *)lockBuffer(Handle)) != NULL)
      {
         Limits->CallerId = CallerId;
         Limits->SearchCriterion = SearchCriterion;
         Limits->profileList = profileList;
         Limits->listSize = listSize; /* for Maximum Profiles */
         Limits->CurrentIndex = *foundCount;  /* for Current number found */
         Limits->RetStatus = SpStatSuccess;   /* Init Good Status */
         Limits->GetALL = FindAll;   /* Find count of all that match */

         /* Loop thru DB entries */
         for (i = 0; i < DataBase->numEntries && 
                     ((Limits->CurrentIndex < Limits->listSize) ||
		      (FindAll > 0)) &&
                     Limits->RetStatus == SpStatSuccess; ++i)
         {
            fileSearch.structSize   = sizeof(KpfileDirEntry_t);
            if (searchMode & SEARCH_NOT_RECURSIVE)
            {
               fileSearch.nwAttr       = ATTR_DIRECTORY; /* No directories */
               fileSearch.wAttr        = 0;
            } else
            {
               fileSearch.nwAttr       = 0;
               fileSearch.wAttr        = 0; /* Take anything found */
            }
            if (searchMode & SEARCH_NOT_RESOLVE_ALIAS)
            	fileSearch.nwAttr |= ATTR_LINK;	/* turn off resolving of alias/shortcut/links */
            
            strcpy(fileSearch.fileName, DataBase->Entries[i].dirName);
            fileSearch.subDirSearch = SubSearch; 

#if defined (KPMAC)
		    fileSearch.osfPrivate.vRefNum      = 
			       DataBase->Entries[i].fileProps.vRefNum;
		    fileSearch.osfPrivate.fileType     = 0;
		    fileSearch.osfPrivate.fileCreator  = 0;
		    fileSearch.osfPrivate.dirID        = 
				   DataBase->Entries[i].fileProps.dirID;

#elif defined (KPWIN32)
		fileSearch.osfPrivate.filter[0] = '\0';
#endif

            /* The Call Back routine tests the individual
             * files reported for the test criteria and if
             * more profiles are found than there is space
             * for, it returns to stop the FileFind routine 
             * and this search loop.
             */
			Fstatus = KpFileFind(&fileSearch,  
                                 Limits, 
                                 CBFunc);
         }
         if (Limits->RetStatus != SpStatSuccess)
         {
            status = Limits->RetStatus;
            *foundCount = 0;
         } else
         {
            *foundCount = Limits->CurrentIndex;
            if (*foundCount < listSize)
               ProfileCount = *foundCount;
            if ((ProfileCount > 0) &&
                (SearchCriterion != (SpSearch_t *)NULL))
            {
				critSize = SearchCriterion->critSize;
				criterion = SearchCriterion->criterion;
				for (i = 0; i < SearchCriterion->critCount; i++)
				{
					if (criterion->SearchElement == SPSEARCH_TIMEORDER) 
					{
						status = SpProfileOrderList(profileList, ProfileCount);
						break;
					}
				}
				criterion = (SpSearchCriterion_t *)(critSize + (KpUInt8_p)criterion);
			}
         }
      }
      else
      	status = SpStatMemory;
      	
      freeBuffer(Handle);
   }
   else
   	status = SpStatMemory;
   	
   return status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Search thru the profiles for those that match the criteria
 *	were loaded.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	October 16, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSearch(
				SpCallerId_t	CallerId,
				SpDataBase_t	*DataBaseList,
                                SpSearch_t	*SearchCriterion,
                                SpProfile_t	*profileList,
                                KpInt32_t	listSize,
                                KpInt32_t	*foundCount)
{
/* Sun returns 2 default diretories so set up for this incase
 * there is a null pointer passed for directories
 */
SpStatus_t		status;
char 			Dir1[MAX_PATH], Dir2[MAX_PATH], Dir3[MAX_PATH];
SpDataBaseEntry_t	List[3];		/* Max num default DBs on any platform */
SpDataBase_t		DataBase;
SpDataBase_p		DB;
KpInt32_t 		dirNameSize;
KpInt32_t		FilesFound = 0;

   status = SpCallerIdValidate (CallerId);
   if (SpStatSuccess != status)
       return status;
		
   /* Null DB entry means get default */
   if (DataBaseList == (SpDataBase_t *)NULL)
   {
      List[0].dirName     = (char *)Dir1;
      List[1].dirName     = (char *)Dir2;
      List[2].dirName     = (char *)Dir3;
      DataBase.numEntries =  GetDefaultDBCount();
      DataBase.Entries    = &List[0];
      dirNameSize = sizeof (Dir1);
      status = SpProfileGetDefaultDB(DataBase.numEntries, dirNameSize, 
                                    DataBase.Entries);
      if (status != SpStatSuccess)
         return status;
      DB = &DataBase;
   } else
      DB = DataBaseList;

   status = SpSearchEngine(CallerId, DB, SearchCriterion, KPFALSE,
			profileList, listSize, &FilesFound, SEARCH_RECURSIVE,
			(KpfileCallBack_t)TestFileCB);

   if (FilesFound < listSize)
      *foundCount = FilesFound;
   else
      *foundCount = listSize;

   return status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Search thru the profiles for those that match the criteria.
 *	Return back count of how many found even if
 *      that number is bigger than the profile list size.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	June 19, 1998
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSearchEx(
				SpCallerId_t	CallerId,
				SpDataBase_t	*DataBaseList,
                                SpSearch_t	*SearchCriterion,
                                SpProfile_t	*profileList,
                                KpInt32_t	listSize,
				KpInt32_t	searchMode,
                                KpInt32_t	*foundCount)
{
/* Sun returns 2 default diretories so set up for this incase
 * there is a null pointer passed for directories
 */
SpStatus_t		status;
char 			Dir1[MAX_PATH], Dir2[MAX_PATH], Dir3[MAX_PATH];
SpDataBaseEntry_t	List[3];		/* Max num default DBs on any platform */
SpDataBase_t		DataBase;
SpDataBase_p		DB;
KpInt32_t 		dirNameSize;
KpInt32_t		FilesFound = 0;

   status = SpCallerIdValidate (CallerId);
   if (SpStatSuccess != status)
       return status;
		
   /* Null DB entry means get default */
   if (DataBaseList == (SpDataBase_t *)NULL)
   {
      List[0].dirName     = (char *)Dir1;
      List[1].dirName     = (char *)Dir2;
      List[2].dirName     = (char *)Dir3;
      DataBase.numEntries =  GetDefaultDBCount();
      DataBase.Entries    = &List[0];
      dirNameSize = sizeof (Dir1);
      status = SpProfileGetDefaultDB(DataBase.numEntries, dirNameSize, 
                                    DataBase.Entries);
      if (status != SpStatSuccess)
         return status;
      DB = &DataBase;
   } else
      DB = DataBaseList;


   searchMode |= SPSEARCHFORALL;
   status = SpSearchEngine(CallerId, DB, SearchCriterion, KPFALSE,
			profileList, listSize, &FilesFound, searchMode,
			(KpfileCallBack_t)TestFileCB);

   *foundCount = FilesFound;

   return status;
}

/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Search thru the profiles for those that match the criteria
 *	and return the count of those files found.  search Mode
 *      allows for recursive searchs.
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	November 10, 1998
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSearchGetListSize(
				SpCallerId_t	CallerId,
				SpDataBase_t	*DataBaseList,
                                SpSearch_t	*SearchCriterion,
				KpInt32_t	searchMode,
                                KpInt32_t	*foundCount)
{
SpProfile_t	*profileList = NULL;
KpInt32_t	listSize = 0;

SpStatus_t		status;

   status = SpProfileSearchEx(CallerId,
				DataBaseList,
				SearchCriterion,
				profileList,
				listSize,
				searchMode,
				foundCount);
		
   return status;
}


/*--------------------------------------------------------------------
 * DESCRIPTION	(Public)
 *	Return the List of Profiles with those that pass the new
 *	search criteria in the front.  foundCount is the number
 *      of Profiles that were found that passed the search criteria
 *
 * AUTHOR
 * 	doro
 *
 * DATE CREATED
 *	November 20, 1995
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSearchRefine(
                                SpSearch_t	*SearchCriterion,
                                SpProfile_t	*profileList,
                                KpInt32_t	listSize,
                                KpInt32_t	*foundCount)
{

SpProfileData_t		*Pf;
SpProfile_t		ThisProfile;
SpStatus_t		status;
KpInt32_t		i, j, critSize;
SpSearchCriterion_t *criterion;

   *foundCount = 0;

   /* Loop thru those profiles given and test header information
    * against search criteria
    */
   for (i = 0; i < listSize; i++)
   {
      Pf = SpProfileLock(profileList[i]);
      if (Pf == NULL)
         return SpStatBadProfile;

      if ((status = SpProfileCheck(SearchCriterion, &Pf->Header)) ==
          SpStatSuccess)
      {
         if (*foundCount == i)
            (*foundCount)++;
         else
         {
            ThisProfile = profileList[i];
            for (j = i; j > *foundCount;  j--)
            {
               profileList[j] = profileList[j - 1];
            }
            profileList[(*foundCount)++] = ThisProfile;
         }
      }
      SpProfileUnlock(profileList[i]);
   }
   if ((*foundCount > 0) &&
       (SearchCriterion != (SpSearch_t *)NULL))
   {
		critSize = SearchCriterion->critSize;
		criterion = SearchCriterion->criterion;
		for (i = 0; i < SearchCriterion->critCount; i++)
		{
			if (criterion->SearchElement == SPSEARCH_TIMEORDER) 
			{
				status = SpProfileOrderList(profileList, *foundCount);
				break;
			}
			criterion = (SpSearchCriterion_t *)(critSize + (KpUInt8_p)criterion);
		}
   }

   return SpStatSuccess;
}

     
