/*
 * @(#)PatchUtils.h	1.23 02/10/22
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// PatchUtils.cpp : Declaration of Patching utilities.
//
// By Stanley Man-Kit Ho


#ifndef __PATCHUTILS_H__
#define __PATCHUTILS_H__


#define BUFFER_SIZE 2048
#define BACKUP_DIRECTORY ".original"

#define VERSION_INFO_FILE "lib\\version.dat"
#define VERSION_INFO_CONTENT "<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>\n<version-info>\n\t<major>%s</major>\n\t<minor>%s</minor>\n\t<micro>%s</micro>\n\t<update>%s</update>\n\t<milestone>%s</milestone>\n\t<build-number>%s</build-number>\n</version-info>"

#define WM_BACKUP_BEGIN	    WM_USER + 100
#define	WM_BACKUP_END	    WM_USER + 101

#define BUFFER_SIZE	    2048


//---------------------------------------------------------------
// Move directory from source to destination
//
// @param lpszSourceDir Path to source directory
// @param lpszDestDir Path to destination directory
// @return TRUE if operation succeeds
//
BOOL MoveDirectory(LPCTSTR lpszSourceDir, LPCTSTR lpszDestDir);

//---------------------------------------------------------------
// Apply patch to a directory
//
// @param lpszDirectory Path to apply the patch
// @param lpszOptions Options passed to patch engine
// @param lpfnCallBack Callback for status
// @return TRUE if operation succeeds
//
BOOL ApplyPatch(LPCTSTR lpszDirectory, LPCTSTR lpszOptions,
		LPVOID (CALLBACK *lpfnCallBack)(UINT Id, LPVOID lpParm));

//---------------------------------------------------------------
// Retrieve patch info
//
// @param lpszDirectory Path to patched directory
// @param lpszVersionInfo Version info
// @return TRUE if operation succeeds
//
BOOL RetrievePatchInfo(LPCTSTR lpszDirectory, LPTSTR lpszVersionInfo);

// Trace facilities
void OPEN_PATCH_TRACEFILE();
void CLOSE_PATCH_TRACEFILE();
void PATCH_TRACE(LPCTSTR, ...);


// Dummy callback
LPVOID WINAPI SilentUpdateCallBack(UINT Id, LPVOID Parm);

// Display information
void DisplayInfo(BOOL bSilentMode, LPCTSTR lpszMessage, LPCTSTR lpszCaption);

// Display error
void DisplayError(BOOL bSilentMode, LPCTSTR lpszMessage, LPCTSTR lpszCaption);

#endif // __PATCHUTILS_H__
