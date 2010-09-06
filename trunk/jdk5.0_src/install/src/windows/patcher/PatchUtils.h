/*
 * @(#)PatchUtils.h	1.23 02/10/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
// Parse version info
//
BOOL ParseVersionString(LPCTSTR lpszVersion, LPTSTR lpszMajorVersion, LPTSTR lpszMinorVersion,
			LPTSTR lpszMicroVersion, LPTSTR lpszUpdateVersion);

//---------------------------------------------------------------
// Create directory with parents
//
// @param lpszDirectory
// return TRUE if operation succeeds
//
BOOL CreateParentDirectory(LPCTSTR lpszDirectory);


//---------------------------------------------------------------
// Move directory from source to destination
//
// @param lpszSourceDir Path to source directory
// @param lpszDestDir Path to destination directory
// @return TRUE if operation succeeds
//
BOOL MoveDirectory(LPCTSTR lpszSourceDir, LPCTSTR lpszDestDir);

//---------------------------------------------------------------
// Copy directory from source to destination
//
// @param lpszSourceDir Path to source directory
// @param lpszDestDir Path to destination directory
// @return TRUE if operation succeeds
//
BOOL CopyDirectory(LPCTSTR lpszSourceDir, LPCTSTR lpszDestDir);

//---------------------------------------------------------------
// Check if directory exists
//
// @param lpszDirectory Path to directory
// @return TRUE if operation succeeds
//
BOOL IsDirectoryExist(LPCTSTR lpszDirectory);

//---------------------------------------------------------------
// Backup directory to directory.original
//
// @param lpszDirectory Path to directory
// @return TRUE if operation succeeds
//
BOOL BackupDirectory(LPCTSTR lpszDirectory);

//---------------------------------------------------------------
// Swap directories
//
// @param lpszDirectory1 Path to directory 1
// @param lpszDirectory2 Path to directory 2
// @return TRUE if operation succeeds
//
BOOL SwapDirectory(LPCTSTR lpszDirectory1, LPCTSTR lpszDirectory2);

//---------------------------------------------------------------
// Remove Directory Recursively
//
// @param lpszDirectory Path to directory to be removed
// @return TRUE if operation succeeds
//
BOOL RemoveDirectoryRecursively(LPCTSTR lpszDirectory);

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
// Create patch info
//
// @param lpszDirectory Path to patched directory
// @return TRUE if operation succeeds
//
BOOL CreatePatchInfo(LPCTSTR lpszDirectory);

//---------------------------------------------------------------
// Retrieve patch info
//
// @param lpszDirectory Path to patched directory
// @param lpszVersionInfo Version info
// @return TRUE if operation succeeds
//
BOOL RetrievePatchInfo(LPCTSTR lpszDirectory, LPTSTR lpszVersionInfo);

//---------------------------------------------------------------
// Check if VM is running
//
// @param lpszDirectory Path to patched directory
// @return TRUE if VM is running
//
BOOL IsVMRunning(LPCTSTR lpszDirectory);


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
