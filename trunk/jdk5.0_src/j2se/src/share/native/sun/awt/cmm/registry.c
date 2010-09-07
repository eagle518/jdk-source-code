/*
 * @(#)registry.c	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains the registry preference functions.

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1995 by Eastman Kodak Company, all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile$
		$Logfile$
		$Revision$
		$Date$
		$Author$

	SCCS Revision:
		@(#)registry.c	1.10 12/22/97

	To Do:
*/
/*********************************************************************/


/**********************************************************************
 **********************************************************************
 **********************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1995                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

#include "kcms_sys.h"
#if !defined(KPMSMAC)
#include <string.h>
#include <stdio.h>

#define TRUEBOOLSTRING  "1"
#define FALSEBOOLSTRING "0"

#define MAINOPENKEY		"Software\\Kodak"
#define MAINOPENKEYINI	"Software\\Kodak\\IniFileMapping"

#define BINFILE			"BinaryFile"
#define BINOFFSET		"Offset"
#define BINBLKSIZE		"BlkSize"

/* local function prototypes */
static KpInt32_t KpCreateRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section);
static KpInt32_t KpOpenRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section);
static KpInt32_t KpOpenReadOnlyRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section);
static KpInt32_t KpWriteRegistry (HKEY hkey, KpChar_p entry, void *buf,
								KpUInt32_t bufSize, DWORD szType);
static KpInt32_t KpReadRegistry (HKEY hkey, KpChar_p entry, void *buf,
								KpUInt32_t *bufSize, DWORD szType);
static KpInt32_t KpCloseRegistry (HKEY hkey);
static KpBool_t KpUseRegistryEx (void);
static void KpconvertSpace (KpChar_p inBuffer, KpChar_p outBuffer);

#if !defined KCMS_MINIMAL_REGISTRY
static KpUInt32_t KpReadSizeRegistry (HKEY hkey, KpChar_p entry);
static KpInt32_t KpDeleteRegistryTree (HKEY hkey, KpChar_p section);
static KpInt32_t KpCreateBinFile (KpChar_p theFile, KpChar_p iniPath,
							KpChar_p section, KpMainHive_t mainHive);
static void KpGetPath (KpChar_p theFile, KpChar_p thePath, KpInt32_t thePathSize);
static KpInt32_t KpWriteBinFile (KpChar_p fileName,
									KpGenericPtr_t data,
									KpInt32_t numBytes,
									KpInt32_t *offset);
static KpInt32_t KpReadBinFile (KpChar_p fileName,
									KpGenericPtr_t data,
									KpInt32_t *numBytes,
									KpInt32_t offset);
static KpInt32_t KpFindRegistryKey (KpChar_p prefsFile, KpChar_p section,
									KpChar_p entry, KpChar_p binfile,
									KpMainHive_t mainHive);
static KpInt32_t KpRemoveBlock (KpChar_p prefsFile, KpChar_p section,
									KpChar_p entry, KpChar_p binfile,
									KpMainHive_t mainHive);
static char * KpCreateSectionName (KpChar_p buf1, KpChar_p buf2, KpChar_p buf3);
#endif


#if !defined KCMS_MINIMAL_REGISTRY

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeleteSectionPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes a section from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpDeleteSectionPreference (KpChar_p prefsFile, ioFileChar_p fileProps,
										KpChar_p section) 
{
	return (KpDeleteSectionPreferenceEx (prefsFile, KPLOCAL_MACHINE, 
											(KpFileProps_p)fileProps, section));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeletePreferences (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes the registry data base tree of the ini file sub-key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * June 16, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpDeletePreferences (KpChar_p prefsFile, ioFileChar_p fileProps) 
{
	return (KpDeletePreferencesEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteBooleanPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a boolean value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteBooleanPreference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpBool_t theBoolean) 
{
	return (KpWriteBooleanPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, 
										resType, resID, section, entry,
										theBoolean));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadBooleanPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a boolean value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadBooleanPreference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpBool_t theDefault, KpBool_p theBoolean) 
{
	return (KpReadBooleanPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, 
									resType, resID, section, entry,
									theDefault, theBoolean));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteInt16Preference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a short value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteInt16Preference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpInt16_t theShort) 
{
  	return (KpWriteInt16PreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps,
  									resType, resID, section, entry, theShort));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadInt16Preference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a short value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadInt16Preference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpInt16_t theDefault, KpInt16_p theShort) 
{
	return (KpReadInt16PreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, resType,
									resID, section, entry, theDefault, theShort));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteInt32Preference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a long value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteInt32Preference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpInt32_t theLong) 
{
 	return (KpWriteInt32PreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, resType,
 									resID, section, entry, theLong));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadInt32Preference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a long value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 21, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadInt32Preference (KpChar_p prefsFile, ioFileChar_p fileProps, 
								KpUInt32_t resType, KpInt16_t resID,
								KpChar_p section, KpChar_p entry,
								KpInt32_t theDefault, KpInt32_p theLong)
{
	return (KpReadInt32PreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, 
									resType, resID, section, entry,
									theDefault, theLong));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteStringPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a string to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteStringPreference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpChar_p theString) 
{
	return (KpWriteStringPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, 
									resType, resID, section, entry,
									theString));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadStringPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a string from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadStringPreference (KpChar_p prefsFile, ioFileChar_p fileProps, 
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpChar_p theDefault, KpChar_p theString,
									KpUInt32_t *theSize) 
{
	return (KpReadStringPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps, 
									resType, resID, section, entry,
									theDefault, theString, theSize));
}
 
/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetStringSizePreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns the string size of the data in the registry.
 *
 * RETURN VALUE
 * 	Returns the string size. If it can't find the string size, returns 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * June 16, 1995
 *-------------------------------------------------------------------*/

KpUInt32_t KpGetStringSizePreference (KpChar_p prefsFile, ioFileChar_p fileProps,
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry)
{
	return (KpGetStringSizePreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps,
									resType, resID, section, entry));
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteBlockPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a block of memory to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 24, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteBlockPreference (KpChar_p prefsFile, ioFileChar_p fileProps,
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpGenericPtr_t theBlock, KpUInt32_t numBytes) 
{
	return (KpWriteBlockPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps,
									resType, resID, section, entry,
									theBlock, numBytes));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadBlockPreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a block of data from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL or KCMS_MORE_DATA.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 24, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadBlockPreference (KpChar_p prefsFile, ioFileChar_p fileProps,
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry,
									KpGenericPtr_t theDefault, KpUInt32_t defnumBytes,
									KpGenericPtr_t theBlock, KpUInt32_t *numBytes) 
{
	return (KpReadBlockPreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps,
									resType, resID, section, entry,
									theDefault, defnumBytes,
									theBlock, numBytes));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetBlockSizePreference (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns the block size of the data in the binary file.
 *
 * RETURN VALUE
 * 	Returns the block size. If it can't find the block size, returns 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 28, 1995
 *-------------------------------------------------------------------*/

KpUInt32_t KpGetBlockSizePreference (KpChar_p prefsFile, ioFileChar_p fileProps,
									KpUInt32_t resType, KpInt16_t resID,
									KpChar_p section, KpChar_p entry) 
{
	return (KpGetBlockSizePreferenceEx (prefsFile, KPLOCAL_MACHINE, (KpFileProps_p)fileProps,
									resType, resID, section, entry));
}


/************** New extended preference functions below *************/

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeleteSectionPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes a section from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpDeleteSectionPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
										KpFileProps_p fileProps, KpChar_p section) 
{
	HKEY hkey, testHkey;
	KpInt32_t	ret;
	DWORD		regRet;
	char buffer[KPPREF_BUF_MAX];

	/* just to humor the compiler */
	if (fileProps){}

	if (prefsFile == NULL)
		return (KCMS_FAIL);

	/* open the main key */
	if (section == NULL) {
		if ((ret = KpOpenRegistry (mainHive, &hkey, MAINOPENKEYINI, NULL, NULL)) != KCMS_SUCCESS)
			return (ret);

		/* strip of the path */
		KpFileStripPath (prefsFile, buffer);
	}
	else {
		if ((ret = KpOpenRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, NULL)) != KCMS_SUCCESS)
			return (ret);

		/* get the section */
		strcpy (buffer, section);
	}

	/* test if entry is in the registry */
	regRet = RegOpenKey (hkey, buffer, &testHkey);
	if (regRet != ERROR_SUCCESS) {
		if (regRet == ERROR_FILE_NOT_FOUND)
			ret = KCMS_SUCCESS;
		else
			ret = KCMS_FAIL;

		goto GetOut;
	}

	/* close the test key */
	KpCloseRegistry (testHkey);

	/* delete the section from the registry */
	ret = KpDeleteRegistryTree (hkey, buffer);

GetOut:
	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeletePreferencesEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes the registry data base tree of the ini file sub-key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpDeletePreferencesEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps) 
{
	return (KpDeleteSectionPreferenceEx (prefsFile, mainHive, fileProps, NULL));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteBooleanPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a boolean value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteBooleanPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpBool_t theBoolean) 
{
	HKEY hkey;
	KpInt32_t	ret;
    char boolType[2];

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((prefsFile == NULL) || (section == NULL))
		return (KCMS_FAIL);

	/* convert the boolean to a string */
	if (theBoolean == KPTRUE)
		strcpy (boolType, TRUEBOOLSTRING);
	else
		strcpy (boolType, FALSEBOOLSTRING);

	/* open/create the main key */
	if ((ret = KpCreateRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section)) != KCMS_SUCCESS)
		return (ret);

	/* write the data out to the registry */
	ret = KpWriteRegistry (hkey, entry, boolType, 2, REG_SZ);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadBooleanPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a boolean value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadBooleanPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpBool_t theDefault,
									KpBool_p theBoolean) 
{
	HKEY hkey;
	char boolType[2];
	KpUInt32_t boolSize = 2;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if (theBoolean == NULL) {
		return (KCMS_FAIL);
	}

	/* open up the main key and get the data from the registry */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section) != KCMS_SUCCESS) {
		goto GetIni;
	}

	if (KpReadRegistry (hkey, entry, boolType, &boolSize, REG_SZ) != KCMS_SUCCESS) {
		/* close the main key */
		KpCloseRegistry (hkey);
		goto GetIni;
	}

	/* convert string to boolean */
	if (!strcmp(TRUEBOOLSTRING, boolType))
		*theBoolean = KPTRUE;
	else
		*theBoolean = KPFALSE;

	/* close the main key */
	KpCloseRegistry (hkey);

	return (KCMS_SUCCESS);

GetIni:
	/* get the data from the ini file instead */
	*theBoolean = (KpInt16_t)GetPrivateProfileInt (section, entry,
											theDefault, prefsFile);

	/* write the ini data out to the registry */
	KpWriteBooleanPreferenceEx (prefsFile, mainHive, fileProps, resType, resID,
								section, entry, *theBoolean);

	return (KCMS_SUCCESS);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteInt16PreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a short value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteInt16PreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpInt16_t theShort) 
{

	/* write the data out to the registry */
  	return (KpWriteInt32PreferenceEx (prefsFile, mainHive, fileProps, resType, resID,
								section, entry, (KpInt32_t)theShort));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadInt16PreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a short value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadInt16PreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpInt16_t theDefault,
									KpInt16_p theShort) 
{
	/* read the data from the registry */
	return (KpReadInt32PreferenceEx (prefsFile, mainHive, fileProps, resType,
									resID, section, entry,
									(KpInt32_t)theDefault, (KpInt32_p)theShort));
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteInt32PreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a long value to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteInt32PreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpInt32_t theLong) 
{

	HKEY hkey;
	KpInt32_t ret;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((prefsFile == NULL) || (section == NULL))
		return (KCMS_FAIL);

	/* open/create the main key */
	if ((ret = KpCreateRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section)) != KCMS_SUCCESS)
		return (ret);

	/* write the data out to the registry */
	ret = KpWriteRegistry (hkey, entry, &theLong, sizeof(KpInt32_t), REG_DWORD);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadInt32PreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a long value from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadInt32PreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpInt32_t theDefault,
									KpInt32_p theLong)
{
	HKEY hkey;
 	char theString[12];
	char theDefaultStr[12];
	KpUInt32_t int32Size = sizeof(KpInt32_t);

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if (theLong == NULL) {
		return (KCMS_FAIL);
	}

	/* open up the main key and get the data from the registry */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section) != KCMS_SUCCESS) {
		goto GetIni;
	}

	if (KpReadRegistry (hkey, entry, theLong, &int32Size, REG_DWORD) !=
			KCMS_SUCCESS) {
		/* close the main key */
		KpCloseRegistry (hkey);
		goto GetIni;
	}
	/* close the main key */
	KpCloseRegistry (hkey);

	return (KCMS_SUCCESS);

GetIni:
    /* get the data from the ini file instead */
    KpItoa (theDefault, theDefaultStr);
	GetPrivateProfileString (section, entry, theDefaultStr, theString,
							12, prefsFile);
	*theLong = KpAtoi (theString);

	/* write the ini data to the registry */
	KpWriteInt32PreferenceEx (prefsFile, mainHive, fileProps, resType, resID,
							section, entry, *theLong);

	return (KCMS_SUCCESS);
}

#endif


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteStringPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a string to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteStringPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpChar_p theString) 
{
	HKEY hkey;
	KpInt32_t ret;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((prefsFile == NULL) || (section == NULL) || (theString == NULL))
		return (KCMS_FAIL);

	/* open/create the main key */
	if ((ret = KpCreateRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section)) != KCMS_SUCCESS)
		return (ret);

	/* write the data out to the registry */
	ret = KpWriteRegistry (hkey, entry, theString, strlen(theString)+1, REG_SZ);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadStringPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a string from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL or KCMS_MORE_DATA.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadStringPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpChar_p theDefault,
									KpChar_p theString, KpUInt32_t *theSize) 
{
	HKEY hkey;
	KpInt32_t ret;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((theString == NULL) || (theSize == NULL)) {
		return (KCMS_FAIL);
	}

	/* open up the main key and get the data from the registry */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section) != KCMS_SUCCESS) {
		goto GetIni;
	}

	ret = KpReadRegistry (hkey, entry, theString, theSize, REG_SZ);
	if (ret != KCMS_SUCCESS) {
		/* close the main key */
		KpCloseRegistry (hkey);

		if (ret == KCMS_FAIL)
			goto GetIni;
		else
			return (ret);
	}
	/* close the main key */
	KpCloseRegistry (hkey);

	return (KCMS_SUCCESS);

GetIni:
	/* get the data from the ini file instead */
	*theSize = GetPrivateProfileString (section, entry, theDefault, theString,
							*theSize, prefsFile) + 1;

	/* write ini data to the registry */
	KpWriteStringPreferenceEx (prefsFile, mainHive, fileProps, resType, resID,
								section, entry, theString);

	return (KCMS_SUCCESS);
}


#if !defined KCMS_MINIMAL_REGISTRY

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetStringSizePreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns the string size of the data in the registry.
 *
 * RETURN VALUE
 * 	Returns the string size. If it can't find the string size, returns 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpUInt32_t KpGetStringSizePreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry)
{
	HKEY hkey;
	KpUInt32_t dataSize;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	/* open up the main key and get the data size from the registry */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, section) != KCMS_SUCCESS)
		return (0);

	dataSize = KpReadSizeRegistry (hkey, entry);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (dataSize);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteBlockPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a block of memory to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 24, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteBlockPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpGenericPtr_t theBlock,
									KpUInt32_t numBytes) 
{
	KpInt32_t	offset, ret = KCMS_SUCCESS;
	KpUInt32_t	thesize = KPPREF_BUF_MAX;
	KpFileProps_t	fileprops;
	KpBool_t		exists;
	char	binfile[KPPREF_BUF_MAX];
	char	*sectionPtr;
	char	filename[KPPREF_BUF_MAX];

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((prefsFile == NULL) || (section == NULL) || (theBlock == NULL))
		return (KCMS_FAIL);

	/* get the binary file name */
	KpReadStringPreferenceEx (prefsFile, mainHive, NULL, 0, 0, section,
							BINFILE, "", binfile, &thesize);
	if (strcmp(binfile, "")) {
		/* does the binary file exist */
		KpFileExists (binfile, NULL, &exists);
		if (exists == KPFALSE) {
			sectionPtr = KpCreateSectionName (section, NULL, NULL);
			if (sectionPtr == NULL)
				return (KCMS_FAIL);

			if (KpDeleteSectionPreferenceEx (prefsFile, mainHive, NULL, sectionPtr) !=
											KCMS_SUCCESS) {
				freeBufferPtr (sectionPtr);
				return (KCMS_FAIL);
			}

			freeBufferPtr (sectionPtr);

			/* write out the binary file name to registry */
			if (KpWriteStringPreferenceEx (prefsFile, mainHive, NULL, 0, 0, section,
										BINFILE, binfile) != KCMS_SUCCESS)
				return (KCMS_FAIL);
		}
		else {
			/* find if duplicate entry */
			if (!KpFindRegistryKey (prefsFile, section, entry, binfile, mainHive)) {

				/* remove duplicate block */
				if (KpRemoveBlock (prefsFile, section, entry, binfile, mainHive) !=
													KCMS_SUCCESS)
					return (KCMS_FAIL);
			}
		}
	}
	else {
		/* create the binary file name */
		if (KpCreateBinFile (binfile, prefsFile, section, mainHive) != KCMS_SUCCESS)
			return (KCMS_FAIL);

		/* delete it if it exists */
		KpFileDelete (binfile, &fileprops);

		/* write out the binary file name to registry */
		KpWriteStringPreferenceEx (prefsFile, mainHive, NULL, 0, 0, 
									section, BINFILE, binfile);
	}

	/* write out to the binary file */
	if (KpWriteBinFile (binfile, theBlock, numBytes, &offset) != KCMS_SUCCESS)
		return (KCMS_FAIL);

	/* create the sub key name */
	KpFileStripPath (binfile, filename);

	/* create the section name */
	sectionPtr = KpCreateSectionName (section, filename, entry);
	if (sectionPtr == NULL)
		return (KCMS_FAIL);

	/* write out the block offset */
	if (KpWriteInt32PreferenceEx (prefsFile, mainHive, NULL, 0, 0, sectionPtr, BINOFFSET,
									offset) != KCMS_SUCCESS) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

	/* write out the block size */
	if (KpWriteInt32PreferenceEx (prefsFile, mainHive, NULL, 0, 0, sectionPtr, BINBLKSIZE,
									numBytes) != KCMS_SUCCESS) {
		ret = KCMS_FAIL;
	}

GetOut:

	freeBufferPtr (sectionPtr);
	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadBlockPreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a block of data from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL or KCMS_MORE_DATA.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpInt32_t KpReadBlockPreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry, KpGenericPtr_t theDefault,
									KpUInt32_t defnumBytes, KpGenericPtr_t theBlock,
									KpUInt32_t *numBytes) 
{
	char binfile[KPPREF_BUF_MAX], filename[KPPREF_BUF_MAX];
	char *sectionPtr = NULL;
	KpUInt32_t retsize, binsize = KPPREF_BUF_MAX;
	KpInt32_t thesize, ret, offset;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	if ((prefsFile == NULL) || (section == NULL) || (theBlock == NULL) ||
		(numBytes == NULL)) {
		return (KCMS_FAIL);
	}

	/* get the binary file name */
	KpReadStringPreferenceEx (prefsFile, mainHive, NULL, 0, 0, section,
							BINFILE, "", binfile, &binsize);

	if (!strcmp(binfile, ""))
		goto GetDefault;

	/* create the sub key name */
	KpFileStripPath (binfile, filename);

	/* create the section name */
	sectionPtr = KpCreateSectionName (section, filename, entry);
	if (sectionPtr == NULL)
		return (KCMS_FAIL);

	/* get the block offset */
	KpReadInt32PreferenceEx (prefsFile, mainHive, NULL, 0, 0, sectionPtr,
							BINOFFSET, -1, &offset);
	if (offset == -1)
		goto GetDefault;

	/* get the block size */
	KpReadInt32PreferenceEx (prefsFile, mainHive, NULL, 0, 0, sectionPtr,
							BINBLKSIZE, 0, &thesize);
	if (thesize == 0)
		goto GetDefault;

 	/* test the block size */
	if ((KpUInt32_t)thesize > *numBytes) {
		retsize = thesize;
		thesize = *numBytes;
		ret = KCMS_MORE_DATA;
	}
	else {
		retsize = thesize;
		ret = KCMS_SUCCESS;
	}

	/* read the data from the binary file */
	if (KpReadBinFile (binfile, theBlock, &thesize, offset) != KCMS_SUCCESS)
		goto GetDefault;

	*numBytes = retsize;

	freeBufferPtr (sectionPtr);

	return (ret);

GetDefault:

	if (sectionPtr != NULL)
		freeBufferPtr (sectionPtr);

	if (theDefault != NULL) {
		KpMemCpy (theBlock, theDefault, defnumBytes);

		/* write the default to the registry */
		KpWriteBlockPreferenceEx (prefsFile, mainHive, fileProps, resType, resID,
								section, entry, theBlock, defnumBytes);
		return (KCMS_SUCCESS);
	}
	else
		return (KCMS_FAIL);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetBlockSizePreferenceEx (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns the block size of the data in the binary file.
 *
 * RETURN VALUE
 * 	Returns the block size. If it can't find the block size, returns 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Oct 09, 1995
 *-------------------------------------------------------------------*/

KpUInt32_t KpGetBlockSizePreferenceEx (KpChar_p prefsFile, KpMainHive_t mainHive,
									KpFileProps_p fileProps, KpUInt32_t resType,
									KpInt16_t resID, KpChar_p section,
									KpChar_p entry) 
{
	char binfile[KPPREF_BUF_MAX], filename[KPPREF_BUF_MAX];
	char *sectionPtr;
	KpUInt32_t numBytes, thesize = KPPREF_BUF_MAX;

	/* just to humor the compiler */
	if (resID){}
	if (resType){}
	if (fileProps){}

	/* get the binary file name */
	KpReadStringPreferenceEx (prefsFile, mainHive, NULL, 0, 0, section,
							BINFILE, "", binfile, &thesize);
	if (!strcmp(binfile, ""))
		return (0);

	/* create the sub key name */
	KpFileStripPath (binfile, filename);

	/* create the section name */
	sectionPtr = KpCreateSectionName (section, filename, entry);
	if (sectionPtr == NULL)
		return (0);

	/* read the block size */
	KpReadInt32PreferenceEx (prefsFile, mainHive, NULL, 0, 0, sectionPtr,
							BINBLKSIZE, 0, (KpInt32_p)&numBytes);
	freeBufferPtr (sectionPtr);

	return (numBytes);
}

#endif

/*************** the following are registry functions ***************/

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteRegistryString (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes a string to the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Jan 24, 1996
 *-------------------------------------------------------------------*/

KpInt32_t KpWriteRegistryString (KpMainHive_t mainHive,	KpChar_p subRegPath,
									KpChar_p stringName, KpChar_p stringBuffer) 
{
	HKEY hkey;
	KpInt32_t ret;
	KpInt8_t subPath[KPPREF_BUF_MAX];

	if ((subRegPath == NULL) || (stringBuffer == NULL))
		return (KCMS_FAIL);

	/* get the main sub path of the registry */
	strcpy (subPath, MAINOPENKEY);

	/* make sure the subRegPath has a '\' */
	if (subRegPath[0] != '\\') {
		strcat (subPath, KcpFileDirSep);
	}

	strcat (subPath, subRegPath);

	/* open/create the main key */
	if ((ret = KpCreateRegistry (mainHive, &hkey, subPath, NULL, NULL)) != KCMS_SUCCESS)
		return (ret);

	/* write the data out to the registry */
	ret = KpWriteRegistry (hkey, stringName, stringBuffer, strlen(stringBuffer)+1, REG_SZ);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadRegistryString (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads a string from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Jan 24, 1996
 *-------------------------------------------------------------------*/

KpInt32_t KpReadRegistryString (KpMainHive_t mainHive, KpChar_p subRegPath,
									KpChar_p stringName, KpChar_p stringBuffer,
									KpUInt32_p bufferSize) 
{
	HKEY hkey;
	KpInt32_t ret;
	KpInt8_t subPath[KPPREF_BUF_MAX];

	if ((subRegPath == NULL) || (stringBuffer == NULL) || (bufferSize == NULL))
		return (KCMS_FAIL);

	/* get the main sub path of the registry */
	strcpy (subPath, MAINOPENKEY);

	/* make sure the subRegPath has a '\' */
	if (subRegPath[0] != '\\') {
		strcat (subPath, KcpFileDirSep);
	}

	strcat (subPath, subRegPath);

	/* open up the main key and get the data from the registry */
	if ((ret = KpOpenReadOnlyRegistry (mainHive, &hkey, subPath, NULL, NULL)) != KCMS_SUCCESS) {
		return (ret);
	}

	ret = KpReadRegistry (hkey, stringName, stringBuffer, bufferSize, REG_SZ);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}


#if !defined KCMS_MINIMAL_REGISTRY

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetRegistryStringSize (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns the string size of the data in the registry.
 *
 * RETURN VALUE
 * 	Returns the string size. If it can't find the string size, returns 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Jan 24, 1996
 *-------------------------------------------------------------------*/

KpUInt32_t KpGetRegistryStringSize (KpMainHive_t mainHive, KpChar_p subRegPath,
									KpChar_p stringName)
{
	HKEY hkey;
	KpUInt32_t dataSize;
	KpInt8_t subPath[KPPREF_BUF_MAX];

	if (subRegPath == NULL)
		return (KCMS_FAIL);

	/* get the main sub path of the registry */
	strcpy (subPath, MAINOPENKEY);

	/* make sure the subRegPath has a '\' */
	if (subRegPath[0] != '\\') {
		strcat (subPath, KcpFileDirSep);
	}

	strcat (subPath, subRegPath);

	/* open up the main key and get the data size from the registry */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, subPath, NULL, NULL) != KCMS_SUCCESS)
		return (0);

	dataSize = KpReadSizeRegistry (hkey, stringName);

	/* close the main key */
	KpCloseRegistry (hkey);

	return (dataSize);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeleteRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes a section from the registry data base.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Jan 30, 1996
 *-------------------------------------------------------------------*/

KpInt32_t KpDeleteRegistry (KpMainHive_t mainHive, KpChar_p subKeyName) 
{
	HKEY hkey, testHkey;
	KpInt32_t	ret;
	DWORD		regRet;
	KpInt8_t	subPath[KPPREF_BUF_MAX];

	if (subKeyName == NULL)
		return (KCMS_FAIL);

	/* make sure the subKeyName has a '\' */
	if (subKeyName[0] == '\\') {
		strcpy (subPath, &subKeyName[1]);
	}
	else
		strcpy (subPath, subKeyName);

	/* open the main key */
	if ((ret = KpOpenRegistry (mainHive, &hkey, MAINOPENKEY, NULL, NULL)) != KCMS_SUCCESS)
		return (ret);

	/* test if entry is in the registry */
	regRet = RegOpenKey (hkey, subPath, &testHkey);
	if (regRet != ERROR_SUCCESS) {
		if (regRet == ERROR_FILE_NOT_FOUND)
			ret = KCMS_SUCCESS;
		else
			ret = KCMS_FAIL;

		goto GetOut;
	}

	/* close the test key */
	KpCloseRegistry (testHkey);

	/* delete the section from the registry */
	ret = KpDeleteRegistryTree (hkey, subPath);

GetOut:
	/* close the main key */
	KpCloseRegistry (hkey);

	return (ret);
}

#endif


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpCreateRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function creates/opens a registry key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpCreateRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section)
{
	char tmpbuf[KPPREF_BUF_MAX], keyname[KPPREF_BUF_MAX];
	DWORD disposition, ret;

	/* setup the key name */
	strcpy (keyname, subRegPath);

	if (prefsFile != NULL) {
		strcpy (keyname, subRegPath);
		strcat (keyname, KcpFileDirSep);
 
		/* strip the path off the ini file */
		KpFileStripPath (prefsFile, tmpbuf);
		strcat (keyname, tmpbuf);
		strcat (keyname, KcpFileDirSep);
	}

	/* open up the key */
	if (!KpUseRegistryEx()) {
		if (section != NULL) {
			/* convert spaces to '_' */
			KpconvertSpace (section, tmpbuf);
			strcat (keyname, tmpbuf);
		}

		if (RegCreateKey (HKEY_CLASSES_ROOT, keyname, hkey) !=
			ERROR_SUCCESS) {
			return (KCMS_FAIL);
		}
	}
	else {
		if (section != NULL) {
			strcat (keyname, section);
		}

		if ((ret = RegCreateKeyEx (mainHive, keyname, 0, NULL,
								   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
								   NULL, hkey, &disposition)) != ERROR_SUCCESS) {
			if (ret == ERROR_ACCESS_DENIED)
				return (KCMS_ACCESS_VIOLATION);
			else
				return (KCMS_FAIL);
		}
	}

	return (KCMS_SUCCESS);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpOpenRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function opens a registry key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpOpenRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section)
{
	char tmpbuf[KPPREF_BUF_MAX], keyname[KPPREF_BUF_MAX];
	DWORD ret;

	/* setup the key name */
	strcpy (keyname, subRegPath);

	if (prefsFile != NULL) {
		/* strip the path off the ini file */
		KpFileStripPath (prefsFile, tmpbuf);
		strcat (keyname, KcpFileDirSep);
		strcat (keyname, tmpbuf);
		if (section != NULL) {
			strcat (keyname, KcpFileDirSep);
			/* convert spaces to '_' */
			if (!KpUseRegistryEx()) {
				KpconvertSpace (section, tmpbuf);
				strcat (keyname, tmpbuf);
			}
			else {
				strcat (keyname, section);
			}
		}
	}

	/* open up the key */
	if (!KpUseRegistryEx()) {
		if (RegOpenKey (HKEY_CLASSES_ROOT, keyname, hkey) !=
			ERROR_SUCCESS) {
			return (KCMS_FAIL);
		}
	}
	else {
		if ((ret = RegOpenKeyEx (mainHive, keyname, 0,
							KEY_ALL_ACCESS, hkey)) != ERROR_SUCCESS) {
			if (ret == ERROR_FILE_NOT_FOUND)
				return (KCMS_ENTRY_NOT_FOUND);
			else
				return (KCMS_FAIL);
		}

	}

	return (KCMS_SUCCESS);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpOpenReadOnlyRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function opens a registry key with ReadOnly permission.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * stanp
 *
 * DATE CREATED
 * March 31, 1999
 *-------------------------------------------------------------------*/

static KpInt32_t KpOpenReadOnlyRegistry (KpMainHive_t mainHive, PHKEY hkey,
									KpChar_p subRegPath, KpChar_p prefsFile,
									KpChar_p section)
{
	char tmpbuf[KPPREF_BUF_MAX], keyname[KPPREF_BUF_MAX];
	DWORD ret;

	/* setup the key name */
	strcpy (keyname, subRegPath);

	if (prefsFile != NULL) {
		/* strip the path off the ini file */
		KpFileStripPath (prefsFile, tmpbuf);
		strcat (keyname, KcpFileDirSep);
		strcat (keyname, tmpbuf);
		if (section != NULL) {
			strcat (keyname, KcpFileDirSep);
			/* convert spaces to '_' */
			if (!KpUseRegistryEx()) {
				KpconvertSpace (section, tmpbuf);
				strcat (keyname, tmpbuf);
			}
			else {
				strcat (keyname, section);
			}
		}
	}

	/* open up the key */
	if (!KpUseRegistryEx()) {
		if (RegOpenKey (HKEY_CLASSES_ROOT, keyname, hkey) !=
			ERROR_SUCCESS) {
			return (KCMS_FAIL);
		}
	}
	else {
		if ((ret = RegOpenKeyEx (mainHive, keyname, 0,
							KEY_READ, hkey)) != ERROR_SUCCESS) {
			if (ret == ERROR_FILE_NOT_FOUND)
				return (KCMS_ENTRY_NOT_FOUND);
			else
				return (KCMS_FAIL);
		}

	}

	return (KCMS_SUCCESS);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes the entries to an open registry key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpWriteRegistry (HKEY hkey, KpChar_p entry, void *buf,
								KpUInt32_t bufSize, DWORD szType)
{
	KpInt32_t	ret;
	KpUInt32_t	dataSize;
	char 		*dataBuf, tmpBuf[KPPREF_BUF_MAX];

	/* set the data */
	if (!KpUseRegistryEx()) {
		switch (szType)
		{
			case REG_DWORD:
				/* allocate memory to hold 10 digit int */
				dataBuf = allocBufferPtr (11);
				if (dataBuf == NULL)
					return (KCMS_FAIL);

				/* convert the data to a string */
				KpItoa ( *(KpInt32_t *)buf, dataBuf);
				dataSize = strlen(dataBuf);
				break;

			case REG_SZ:
				/* allocate memory to hold a string */
				dataBuf = allocBufferPtr (bufSize);
				if (dataBuf == NULL)
					return (KCMS_FAIL);

				strcpy (dataBuf, buf);
				dataSize = strlen(dataBuf);
				break;

			case REG_EXPAND_SZ:
			case REG_LINK:
			case REG_MULTI_SZ:
			case REG_NONE:
			case REG_RESOURCE_LIST:
			default:
				return (KCMS_FAIL);
		}
		/* convert spaces to '_' */
		KpconvertSpace (entry, tmpBuf);

		/* write out the string */
		ret = RegSetValue (hkey, tmpBuf, REG_SZ, dataBuf, dataSize);

		/* free the buffer */
		freeBufferPtr (dataBuf);

		return (ret);
	}
	else {
			
		if (RegSetValueEx (hkey, entry, 0, szType, (BYTE *)buf, bufSize) !=
		 	ERROR_SUCCESS)
		 	return (KCMS_FAIL);
		else
			return (KCMS_SUCCESS);
	}
} 

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads the entries from an open registry key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpReadRegistry (HKEY hkey, KpChar_p entry, void *buf,
							KpUInt32_t *bufSize, DWORD szType)
{
	char *dataBuf, tmpBuf[KPPREF_BUF_MAX];
	DWORD szRead;
	KpInt32_t *intptr, ret = KCMS_SUCCESS;
	KpUInt32_t dataSize;
	LONG	regRet;

	/* set the data */
	if (!KpUseRegistryEx()) {
		
		/* convert spaces to '_' */
		KpconvertSpace (entry, tmpBuf);

		/* get the string size */
		regRet = RegQueryValue (hkey, tmpBuf, NULL, (PLONG)&dataSize);
		if (regRet != ERROR_SUCCESS)
			return (KCMS_FAIL);

		/* allocate memory for the return string */
		dataBuf = allocBufferPtr (dataSize);
		if (dataBuf == NULL)
			return (KCMS_FAIL);

		/* get the data string */
		regRet = RegQueryValue (hkey, tmpBuf, dataBuf, (PLONG)&dataSize);
		if (regRet != ERROR_SUCCESS) {
			ret = KCMS_FAIL;
			goto GetOut;
		}

		switch (szType)
		{
			case REG_DWORD:
				/* convert the data */
				intptr = buf;
				*intptr = KpAtoi (dataBuf);
				break;

			case REG_SZ:
				if (dataSize <= *bufSize)
					strcpy (buf, dataBuf);
				else
					ret = KCMS_MORE_DATA;

				*bufSize = dataSize;
				break;

			case REG_EXPAND_SZ:
			case REG_LINK:
			case REG_MULTI_SZ:
			case REG_NONE:
			case REG_RESOURCE_LIST:
			default:
				ret = KCMS_FAIL;
				break;
		}

GetOut:
		/* free the buffer */
		freeBufferPtr (dataBuf);

		return (ret);
	}
	else {
			
		regRet = RegQueryValueEx (hkey, entry, NULL, &szRead,
								(BYTE *)buf, (DWORD *)bufSize);
		if (regRet != ERROR_SUCCESS) {
			if (regRet == ERROR_MORE_DATA)
				return (KCMS_MORE_DATA);
			else
		 		return (KCMS_FAIL);
		}

		if (szRead != szType)
			return (KCMS_FAIL);
		else
			return (KCMS_SUCCESS);
	}
} 


#if !defined KCMS_MINIMAL_REGISTRY

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadSizeRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads the size of an open registry key's data.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is the size. If
 *  the function fails, the return value is 0.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpUInt32_t KpReadSizeRegistry (HKEY hkey, KpChar_p entry)
{
	DWORD szRead;
	KpUInt32_t dataSize;
	LONG	regRet;

	/* set the data */
	if (!KpUseRegistryEx()) {
		
		/* get the data size */
		regRet = RegQueryValue (hkey, entry, NULL, (PLONG)&dataSize);
		if (regRet != ERROR_SUCCESS)
			return (0);
		else
			return (dataSize);
	}
	else {
			
		regRet = RegQueryValueEx (hkey, entry, NULL, &szRead,
								NULL, (LPDWORD)&dataSize);
		if (regRet != ERROR_SUCCESS)
		 		return (0);
		else
			return (dataSize);
	}
} 

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpDeleteRegistryTree (WIN32 Version)
 *
 * DESCRIPTION 
 * This function deletes a section in an open registry key.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * May 4, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpDeleteRegistryTree (HKEY hkey, KpChar_p section)
{
	HKEY sectionhkey;
	char buffer[KPPREF_BUF_MAX];
	DWORD ret;

	/* open up the section key */	
	if (RegOpenKey (hkey, section, &sectionhkey) != ERROR_SUCCESS) {
		return (KCMS_FAIL);
	}

	for (;;) {
		ret = RegEnumKey (sectionhkey, 0, buffer, KPPREF_BUF_MAX);
		if (ret == ERROR_SUCCESS) {
			ret = KpDeleteRegistryTree (sectionhkey, buffer);
			if (ret != KCMS_SUCCESS)
				return (ret);
		}
		else {
			KpCloseRegistry (sectionhkey);
			ret = RegDeleteKey (hkey, section);
			if (ret != ERROR_SUCCESS)
				return (KCMS_FAIL);
			else
				return (KCMS_SUCCESS);
		}
	}
}

#endif


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpCloseRegistry (WIN32 Version)
 *
 * DESCRIPTION 
 * This function closes the opened registry key. 
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value ERROR_SUCCESS. If the
 *	function fails, the return value is an error value.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpCloseRegistry (HKEY hkey)
{
	return (RegCloseKey(hkey));
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpUseRegistryEx (WIN32 Version)
 *
 * DESCRIPTION 
 *	This function examines what version of Windows is running and
 *	returns TRUE if that version of Windows supports the extended
 *	WIN32 API functions.  
 *	Otherwise it returns FALSE
 *
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * Feb 16, 1995
 *-------------------------------------------------------------------*/

static KpBool_t KpUseRegistryEx (void)
{
	static KpBool_t	Initialize = KPTRUE;
	static KpBool_t	UseRegEx = KPFALSE;
	KpOsType_t	wintype;
	KpInt32_t version;

	if (Initialize) {
		Initialize = KPFALSE;
		KpGetSystemInfo (&wintype, &version);

		if (wintype == KPOSTYPE_WIN32S)
			UseRegEx = KPFALSE;
		else
			UseRegEx = KPTRUE;
	}
	return UseRegEx;
}


#if !defined KCMS_MINIMAL_REGISTRY

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpGetPath (WIN32 Version)
 *
 * DESCRIPTION 
 * This function returns a path. 
 *
 * RETURN VALUE
 *  None.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 24, 1995
 *-------------------------------------------------------------------*/

static void KpGetPath (KpChar_p theFile, KpChar_p thePath, KpInt32_t thePathSize)
{
	char *fPtr;

	strcpy (thePath, theFile);

	/* find the last occurrence of the backslash */
	fPtr = strrchr (thePath, '\\');
	if (fPtr == NULL) {
		/* find the occurrence of the ':' */
		fPtr = strrchr (thePath, ':');
		if (fPtr == NULL) {
			if (GetWindowsDirectory (thePath, thePathSize) == 0)
				thePath[0] = '\0';
			else {
				strcat (thePath, "\\kpcms\\");
				if (CreateDirectory (thePath, NULL) != KPTRUE) {
					if (GetLastError () != ERROR_ALREADY_EXISTS)
						thePath[0] = '\0';
				}
			}
		}
		else {
			*fPtr++;
			*fPtr = '\0';
		}
	}
	else {
		*fPtr++;
		*fPtr = '\0';
	}
}
 

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpCreateBinFile (WIN32 Version)
 *
 * DESCRIPTION 
 * This function creates a binary file name. 
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If
 *  the function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 21, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpCreateBinFile (KpChar_p theFile, KpChar_p iniPath,
							KpChar_p section, KpMainHive_t mainHive)
{
	char buffer[KPPREF_BUF_MAX], inifile[KPPREF_BUF_MAX], path[KPPREF_BUF_MAX];
	char userName[25];
	DWORD	userSize;
	KpCrc32_t	crc;

	/* get the path */
	KpGetPath (iniPath, path, KPPREF_BUF_MAX);

	/* strip off the path */
	KpFileStripPath (iniPath, inifile);

	/* combined the strings */
	if (KpUseRegistryEx()) { /* windows NT/95 */
		/* get the main hive value */
		sprintf (buffer, "%Ix", mainHive);

		/* combined the strings */
		strcat (buffer, inifile);
		strcat (buffer, section);

		/* get the user name */
		if (mainHive == KPCURRENT_USER) {
			userSize = 25;
			if (GetUserName (userName, &userSize) != TRUE)
				return (KCMS_FAIL);

			strcat (buffer, userName);
		}
	}
	else { /* win32s */
		strcpy (buffer, inifile);
		strcat (buffer, section);
	}

	/* calculate the crc of the string */
	crc = Kp_Crc32 (0, (KpInt32_t)strlen(buffer), buffer);

	/* create the new binary file name */
	sprintf (buffer, "%x", crc);
	strcpy (theFile, path);
	strcat (theFile, buffer);
	strcat (theFile, ".bin");

	return (KCMS_SUCCESS);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpWriteBinFile (WIN32 Version)
 *
 * DESCRIPTION 
 * This function writes data to the binary file. 
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If the
 *	function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 25, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpWriteBinFile (KpChar_p fileName,
									KpGenericPtr_t data,
									KpInt32_t numBytes,
									KpInt32_t *offset)
{
	KpFileId	fd;
	KpInt32_t	ret = KCMS_SUCCESS;

 	/* open up the binary file */
	if (KpFileOpen (fileName, "W+", NULL, &fd) == 0)
		return (KCMS_FAIL);

	/* goto end of the file */
	if (KpFilePosition (fd, FROM_END, 0) == KCMS_IO_ERROR) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

	/* get the offset of the new block */
	if (KpFileTell (fd, offset) == 0) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

	/* write out the data */
	if (KpFileWrite (fd, data, numBytes) == 0) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

GetOut:
	/* close the binary file */
	KpFileClose (fd);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpReadBinFile (WIN32 Version)
 *
 * DESCRIPTION 
 * This function reads data from the binary file. 
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If the
 *	function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 25, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpReadBinFile (KpChar_p fileName,
									KpGenericPtr_t data,
									KpInt32_t *numBytes,
									KpInt32_t offset)
{
	KpFileId	fd;
	KpInt32_t	ret = KCMS_SUCCESS;

 	/* open up the binary file */
	if (KpFileOpen (fileName, "R", NULL, &fd) == 0)
		return (KCMS_FAIL);

	/* goto end of the file */
	if (KpFilePosition (fd, FROM_START, offset) == KCMS_IO_ERROR) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

	/* read the data */
	if (KpFileRead (fd, data, numBytes) == 0) {
		ret = KCMS_FAIL;
		goto GetOut;
	}

GetOut:
	/* close the binary file */
	KpFileClose (fd);

	return (ret);
}

/*--------------------------------------------------------------------
 * FUNCTION
 *	KpFindRegistryKey (WIN32 Version)
 *
 * DESCRIPTION 
 * This function looks for a duplicate sub-key. 
 *
 * RETURN VALUE
 * 	If the function finds the key, the return value is 0. If the
 *	function can't find the key, the return value is 1.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 25, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpFindRegistryKey (KpChar_p prefsFile, KpChar_p section,
									KpChar_p entry, KpChar_p binfile,
									KpMainHive_t mainHive)
{
	HKEY hkey;
	char buffer[KPPREF_BUF_MAX], filename[KPPREF_BUF_MAX];
	char *sectionPtr;
	DWORD ret, i = 0, found = 0;

	/* create the sub key name */
	KpFileStripPath (binfile, filename);

	/* create the section name */
	sectionPtr = KpCreateSectionName (section, filename, NULL);
	if (sectionPtr == NULL)
		return (1);

	/* open up the main key */
	if (KpOpenReadOnlyRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, sectionPtr) != KCMS_SUCCESS) {
		freeBufferPtr (sectionPtr);
		return (1);
	}

	/* enum the sub keys if any */
	do
	{
		if ((ret = RegEnumKey (hkey, i++, buffer, KPPREF_BUF_MAX)) ==
				ERROR_SUCCESS) {
			if (!strcmp(buffer, entry)) {
				found++;
			}
		}
	} while (ret == ERROR_SUCCESS);

	/* close the section key */
	KpCloseRegistry (hkey);

	freeBufferPtr (sectionPtr);

	if (!found)
		return (1);
	else
		return (0);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpRemoveBlock (WIN32 Version)
 *
 * DESCRIPTION 
 * This function removes the duplicate entry in the binary file. 
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is KCMS_SUCCESS. If the
 *	function fails, the return value is KCMS_FAIL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * April 25, 1995
 *-------------------------------------------------------------------*/

static KpInt32_t KpRemoveBlock (KpChar_p prefsFile, KpChar_p section,
									KpChar_p entry, KpChar_p binfile,
									KpMainHive_t mainHive)
{
	HKEY hkey, entryhkey;
	char buffer[KPPREF_BUF_MAX], tempfile[KPPREF_BUF_MAX];
	char filename[KPPREF_BUF_MAX], *sectionPtr;
	DWORD ret, i = 0;
	KpInt32_t	offset, numbytes;
	KpUInt32_t	int32Size;
	KpGenericPtr_t dPtr = NULL;
	KpFileProps_t	fileprops;
	KpBool_t	exists;

	/* create the temp file name */
	KpGetPath (binfile, tempfile, KPPREF_BUF_MAX);
	strcat (tempfile, "~kptmp.tmp");

	/* if old version of the temp file exists, delete it */
	KpFileExists (tempfile, NULL, &exists);
	if (exists == KPTRUE)
		KpFileDelete (tempfile, &fileprops);


	/* create the sub key name */
	KpFileStripPath (binfile, filename);

	/* create the section name */
	sectionPtr = KpCreateSectionName (section, filename, NULL);
	if (sectionPtr == NULL)
		return (KCMS_FAIL);

	/* open up the main key */
	if (KpOpenRegistry (mainHive, &hkey, MAINOPENKEYINI, prefsFile, sectionPtr) != KCMS_SUCCESS) {
		freeBufferPtr (sectionPtr);
		return (KCMS_FAIL);
	}

	freeBufferPtr (sectionPtr);

	/* enum the sub keys if any */
	do
	{
		if ((ret = RegEnumKey (hkey, i++, buffer, KPPREF_BUF_MAX)) ==
				ERROR_SUCCESS) {
			if (strcmp(buffer, entry)) {
				/* open the sub-key */
				if (RegOpenKey (hkey, buffer, &entryhkey) != ERROR_SUCCESS) {
					KpCloseRegistry (hkey);
					return (KCMS_FAIL);
				}

				/* read the offset value */
				int32Size = sizeof(KpInt32_t);
				if (KpReadRegistry (entryhkey, BINOFFSET, &offset,
								&int32Size, REG_DWORD) != KCMS_SUCCESS)
					goto GetOut;

				/* read the byte size value */
				int32Size = sizeof(KpInt32_t);
				if (KpReadRegistry (entryhkey, BINBLKSIZE, &numbytes,
								&int32Size, REG_DWORD) != KCMS_SUCCESS)
					goto GetOut;

				/* alloc some memory to hold the block */
				dPtr = allocBufferPtr (numbytes);
				if (dPtr == NULL)
					goto GetOut;

				/* read the data from the main binary file */
				if (KpReadBinFile (binfile, dPtr, &numbytes,
									offset) != KCMS_SUCCESS)
					goto GetOut;

				/* write the data to the temp binary file */
				if (KpWriteBinFile (tempfile, dPtr, numbytes,
									&offset) != KCMS_SUCCESS)
					goto GetOut;

				/* write the offset value */
				if (KpWriteRegistry (entryhkey, BINOFFSET, &offset,
								sizeof(KpInt32_t), REG_DWORD) != KCMS_SUCCESS)
					goto GetOut;

				/* free the buffer */
				freeBufferPtr (dPtr);
				dPtr = NULL;

				/* close the sub-key */
				KpCloseRegistry (entryhkey);
			}
		}
	} while (ret == ERROR_SUCCESS);

	/* delete the old binary file */
	KpFileDelete (binfile, &fileprops);

	/* rename the the temp file */
	KpFileRename (&fileprops, tempfile, binfile);

	/* close the section key */
	KpCloseRegistry (hkey);

	return (KCMS_SUCCESS);

GetOut:
	/* free the buffer */
	if (dPtr != NULL)
		freeBufferPtr (dPtr);

	/* close the sub-key */
	KpCloseRegistry (entryhkey);

 	/* close the section key */
	KpCloseRegistry (hkey);

	return (KCMS_FAIL);
}


/*--------------------------------------------------------------------
 * FUNCTION
 *	KpCreateSectionName (WIN32 Version)
 *
 * DESCRIPTION 
 * This function allocates a buffer to hold the key names.
 *
 * RETURN VALUE
 * 	If the function succeeds, the return value is a valid pointer. If
 *  the function fails, the return value is NULL.
 * 
 * AUTHOR
 * sek
 *
 * DATE CREATED
 * May 1, 1995
 *-------------------------------------------------------------------*/

static char * KpCreateSectionName (KpChar_p buf1, KpChar_p buf2, KpChar_p buf3)
{
	KpInt32_t totalSize = 0;
	static char *buffer;

	if (buf1 != NULL)
		totalSize += strlen(buf1);
	
	if (buf2 != NULL)
		totalSize += strlen(buf2) + 1;
	
	if (buf3 != NULL)
		totalSize += strlen(buf3) + 1;

	buffer = allocBufferPtr (totalSize+1);
	if (buffer == NULL)
		return (NULL);

	if (buf1 != NULL)
		strcpy (buffer, buf1);
	
	if (buf2 != NULL) {
		strcat (buffer, "\\");
		strcat (buffer, buf2);
	}
	
	if (buf3 != NULL) {
		strcat (buffer, "\\");
		strcat (buffer, buf3);
	}

	return (buffer);
}

#endif


static void KpconvertSpace (KpChar_p inBuffer, KpChar_p outBuffer)
{
	KpUInt32_t i, len;

	/* test the buffer */
	if ((inBuffer == NULL) || (outBuffer == NULL))
		return;

	/* get the size of the string */
	len = strlen (inBuffer);

	/* remove any space characters */
	for (i = 0; i < len; i++) {
		if (inBuffer[i] == ' ') {
			outBuffer[i] = '_';
		}
		else {
			outBuffer[i] = inBuffer[i];
		}
	}

	outBuffer[i] = '\0';
}


#endif
