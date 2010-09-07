/*
 * @(#)filemap.c	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*********************************************************************/
/*
	Contains:	This module contains portable file mapping functions.

				Created by lsh, November 9, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1997 by Eastman Kodak Company, 
			    all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: filemap.c $
		$Logfile: /DLL/KodakCMS/kpsys_lib/filemap.c $
		$Revision: 3 $
		$Date: 5/09/02 2:46p $
		$Author: Msm $

	SCCS Revision:
		@(#)filemap.c	1.16 12/4/98

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

#include "kcms_sys.h"

#if defined (KPMAC)
#include <string.h>
#endif

#if defined (KPWIN32) && !defined(KPMSMAC)


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Map an entire file. (WIN32 Version)
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
void FAR *KpMapFileEx (
				KpChar_p		FileName,
				KpFileProps_t	FAR *FileProps,
				KpChar_p		Mode,
				KpMapFile_t	FAR *MapFileCtl)
{
	KpInt32_t	status;
	DWORD		fdwProtect;
	DWORD		fdwAccess;

/* validate mode */
	switch (*Mode) {
	case 'R':
	case 'r':
		fdwProtect = PAGE_READONLY;
		fdwAccess = FILE_MAP_READ;
		break;
	
#if 0
	case 'w':
		fdwProtect = PAGE_READWRITE;
		fdwAccess = FILE_MAP_WRITE;
		break;
#endif
	
	default:
		return NULL;
	}

/* get the size of the file */
	if (!KpFileSize (FileName, FileProps, &MapFileCtl->NumBytes))
		return NULL;
	
/* open the file */
	status = KpFileOpen (FileName, Mode, FileProps, (KpFileId *)&MapFileCtl->hFile);

	if (KCMS_IO_SUCCESS != status)
		return NULL;

/* create the mapping object */
	MapFileCtl->hMapObject = CreateFileMapping ((HANDLE)MapFileCtl->hFile,
												NULL, fdwProtect, 0, 0, NULL);
	if (!MapFileCtl->hMapObject) {
		KpFileClose((KpFileId)MapFileCtl->hFile);
		return NULL;
	} 

/* create a view of the file */
	MapFileCtl->Ptr = MapViewOfFile (MapFileCtl->hMapObject,
										fdwAccess, 0, 0, 0);
	if (NULL == MapFileCtl->Ptr) {
		CloseHandle (MapFileCtl->hMapObject);
		KpFileClose((KpFileId)MapFileCtl->hFile);
		return NULL;
	}

/* return the address of the memory mapped file */
	return MapFileCtl->Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Map an entire file. (WIN32 Version)
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
void FAR *KpMapFile (
				KpChar_p		FileName,
				ioFileChar	FAR *FileProps,
				KpChar_p		Mode,
				KpMapFile_t	FAR *MapFileCtl)
{
	/* just call KpMapFileEx */
	return ( KpMapFileEx(FileName, (KpFileProps_t FAR *)FileProps,
			Mode, MapFileCtl) );
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Unmap an entire file. (WIN32 Version)
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
KpInt32_t KpUnMapFile (KpMapFile_t FAR *MapFileCtl)
{
	int	Status;

	Status = KCMS_IO_SUCCESS;
	if (!UnmapViewOfFile (MapFileCtl->Ptr)) {
		Status = KCMS_IO_ERROR;
	} else {
		if (!CloseHandle (MapFileCtl->hMapObject)) {
			Status = KCMS_IO_ERROR;
		} else {
			Status = KpFileClose((KpFileId)MapFileCtl->hFile);
		}
	}
	return Status;
}

#else


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Map an entire file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
void FAR *KpMapFileEx (
				KpChar_p		FileName,
				KpFileProps_t	FAR *FileProps,
				KpChar_p		Mode,
				KpMapFile_t	FAR *MapFileCtl)
{
/* validate mode */
	switch (*Mode) {
	case 'R':
	case 'r':
#if 0
	case 'w':
#endif
		break;
	
	default:
		return NULL;
	}

/* get the size of the file */
	if (!KpFileSize (FileName, FileProps, &MapFileCtl->NumBytes))
		return NULL;

/* allocate buffer to hold file */
	MapFileCtl->Ptr = allocBufferPtr (MapFileCtl->NumBytes);
	if (NULL == MapFileCtl->Ptr)
		return NULL;

/* open the file */
	if (!KpFileOpen (FileName, Mode, FileProps, &MapFileCtl->Fd)) {
		freeBufferPtr (MapFileCtl->Ptr);
		return NULL;
	}

/* read file into the buffer */
	if (!KpFileRead (MapFileCtl->Fd, MapFileCtl->Ptr, &MapFileCtl->NumBytes)) {
		freeBufferPtr (MapFileCtl->Ptr);
		KpFileClose (MapFileCtl->Fd);
		return NULL;
	}

/* done with file, close it */
	if ('r' == *Mode) {
		KpFileClose (MapFileCtl->Fd);
		MapFileCtl->Fd = -1;
	}
	MapFileCtl->Mode = *Mode;

	return MapFileCtl->Ptr;
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Map an entire file. (non-win32 version)
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
void FAR *KpMapFile (
				KpChar_p		FileName,
				ioFileChar	FAR *FileProps,
				KpChar_p		Mode,
				KpMapFile_t	FAR *MapFileCtl)
{
KpFileProps_t	kpFileProps, *kpFilePropsPtr;

	/* convert FileProps to KpFileProps_t and call
	   KpMapFileEx */
#if defined (KPMAC) || defined (KPMSMAC)
	if (FileProps != NULL) {
		kpFileProps.vRefNum = FileProps->vRefNum;
		kpFileProps.dirID= 0;
		strncpy(kpFileProps.fileType, FileProps->fileType, 5);
		strncpy(kpFileProps.creatorType, FileProps->creatorType, 5);
	}
	else {
		kpFileProps.vRefNum = 0;
		kpFileProps.dirID= 0;
		strncpy(kpFileProps.fileType, "    ", 5);
		strncpy(kpFileProps.creatorType, "    ", 5);
	}
	kpFilePropsPtr = &kpFileProps;
#else
	kpFilePropsPtr = FileProps;
#endif
	return (KpMapFileEx (FileName, kpFilePropsPtr, Mode, MapFileCtl) );
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Unmap an entire file.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	November 9, 1993
 *------------------------------------------------------------------*/
KpInt32_t KpUnMapFile (KpMapFile_t FAR *MapFileCtl)
{
	KpInt32_t Status;

	Status = KCMS_IO_SUCCESS;
    if ('w' == MapFileCtl->Mode) {
		if (KpFilePosition (MapFileCtl->Fd, FROM_START, 0)) {
			if (!KpFileWrite (MapFileCtl->Fd, MapFileCtl->Ptr,
												MapFileCtl->NumBytes))
				Status = KCMS_IO_ERROR;
		}
		else
			Status = KCMS_IO_ERROR;
		KpFileClose (MapFileCtl->Fd);
	}

	freeBufferPtr (MapFileCtl->Ptr);
	return Status;
}
#endif
