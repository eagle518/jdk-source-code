/*
 * @(#)jinstall.h	1.42 09/04/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// JInstall.h by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
// Contains WinMain of the JInstall
//

#ifndef _JINSTALL_H_
#define _JINSTALL_H_

#define REQUIRED_TEMP_DISKSPACE 15  //MB

BOOL RemoveFile(LPCTSTR lpszFilePath);
BOOL InstallIntlJRE();
BOOL SetMSTFileNames(BOOL bSponsor, LPSTR pszMSTCachedFile, LPSTR pszMSTSrcURL);
BOOL ParseExistUpdateVersion(LPCTSTR lpszVersion, LPTSTR lpszUpdateVersion);

DWORD ExecuteInstallation(LPCTSTR lpApplicationName, LPTSTR lpszCommandOptions, LPCTSTR lpszOnlyFirstCommandOptions, LPCTSTR lpszMST);
DWORD UninstallSixUTenBeta();
void DownloadFailure();

void SetPostStatus(BOOL bFlag);
void SetPostStatusURL(LPCTSTR lpszURL);
void SetCountryLookupURL(LPCTSTR lpszURL);
void SetPreferenceEngineOrder(LPCTSTR lpszPreference);
void SetVPreloadURLValue(LPCTSTR lpszVPreloadURL);
void SetVRunURLValue(LPCTSTR lpszVRunURL);
void SetVMinOSValue(LPCTSTR lpszVMinOS);
void SetVMinPSpeedValue(LPCTSTR lpszVMinPSpeed);
void SetVMinMemoryValue(LPCTSTR lpszVMinMemory);
void SetVLocaleValue(LPCTSTR lpszVLocale);
void SetVGeosValue(LPCTSTR lpszVGeos);
void SetVAuxNameValue(LPCTSTR lpszAuxName);
void DeleteRegVectorURL();
BOOL IsStringinXMLList(LPCTSTR szList, LPCTSTR szLocalStr);
BOOL IsMinProcessSpeed();
BOOL IsMinMemorySize();
BOOL IsJavaFXKeyFound();

BOOL FinalizeRunOnce();
BOOL IsAuxInstalled();
BOOL IsVectorInstall();
BOOL IsVectorRunOnce();
BOOL IsInstallFromJavaUpdate();
BOOL OverrideOptions();
BOOL IsWinXpOrLater();
BOOL IsMinOS();
BOOL IsFilesDownloaded(LPCTSTR szSinglemsiCachedFile, LPCTSTR szMSTCachedFile,
                        LPSTR szMessage);
#endif // _JINSTALL_H_

