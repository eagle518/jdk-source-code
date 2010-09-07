/*
 * @(#)wrapper.cpp	1.6 10/03/31
 * 
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#include "StdAfx.h"
#include <process.h>
#include <tchar.h>
#include "resource.h"
#include <wininet.h>
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "UserProfile.h"
#include "WrapperUtils.h"



CComModule _Module;

#define BUFFER_SIZE 1024


//Get the destination location for the extracted installer
void GetCobundleFileName(LPTSTR lpszLocalFileName, LPTSTR lpszLocalDirName, LPTSTR lpszCobundleName) {
    getUserHome(lpszLocalDirName);
    wsprintf(lpszLocalDirName, "%s\\%s%s%_combo", lpszLocalDirName, BUNDLE, VERSION);

    CreateDirectory(lpszLocalDirName, NULL);
    wsprintf(lpszLocalFileName, "%s\\%s", lpszLocalDirName, lpszCobundleName);
}

int ExecCommand(LPSTR lpszCommand) {
    PROCESS_INFORMATION pf;
    STARTUPINFO sf;
    DWORD dwExitCode = 1;

    ::ZeroMemory(&pf, sizeof(pf));
    ::ZeroMemory(&sf, sizeof(sf));

    if(::CreateProcess(NULL, lpszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &sf, &pf)) {
        ::WaitForSingleObject(pf.hProcess, INFINITE);
	GetExitCodeProcess(pf.hProcess, &dwExitCode);
	::CloseHandle(pf.hProcess);
    }

    return dwExitCode;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
	             LPTSTR     lpCmdLine,
                     int       nCmdShow) {
    lpCmdLine = GetCommandLine(); 
    LANGID gLangid;
    TCHAR sz32bitExeCommand[MAX_PATH] = {0} ;
    TCHAR sz64bitExeCommand[MAX_PATH] = {0} ;
    TCHAR sz32bitExeName[MAX_PATH] = {0} ;
    TCHAR sz64bitExeName[MAX_PATH] = {0} ;
    TCHAR sz32ExecutableName[MAX_PATH] = {0} ;
    TCHAR sz64ExecutableName[MAX_PATH] = {0} ;
    TCHAR command[BUFFER_SIZE] = {0} ;
    TCHAR szDirName[BUFFER_SIZE] = {0} ;


    int i64bitError = ERROR_SUCCESS;
    int i32bitError = ERROR_SUCCESS;

    for (int i=1; i< __argc; i++) {
            lstrcat(command, __argv[i]);
            lstrcat(command, " ");
    }

    wsprintf(sz32bitExeName, JRE_32BIT_INSTALLER);
    wsprintf(sz64bitExeName, JRE_64BIT_INSTALLER);


    //Verify if it's 64-bit OS before extraction of 64-bit installer
    if(IsSystem64bit()) {
        GetCobundleFileName(sz64ExecutableName, szDirName, sz64bitExeName);

        if (ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_64), 
				"JAVA_64_INSTALLER", sz64ExecutableName)) {

        wsprintf(sz64bitExeCommand, "%s %s", sz64ExecutableName, command);
        i64bitError = ExecCommand(sz64bitExeCommand);
 
        }
       ::DeleteFile(sz64ExecutableName);
    }



    //Get the destination directory and file path for the extracted 32-bit installer
    GetCobundleFileName(sz32ExecutableName, szDirName,  sz32bitExeName);

    if (ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_32), 
				"JAVA_32_INSTALLER", sz32ExecutableName)) {
    //Form the installer command
    wsprintf(sz32bitExeCommand, "%s %s", sz32ExecutableName, command);

    //Install 32-bit jre
    i32bitError = ExecCommand(sz32bitExeCommand);

    }

    //Remove the extracted installer executable file
    ::DeleteFile(sz32ExecutableName);


    //Remove the combo directory 
    ::RemoveDirectory(szDirName);

    //if the 32-bit install fails return its error code, otherwise return the 64-bit error code.
    if (i32bitError != 0) {
        return i32bitError;
    }else {
        return i64bitError;
    }
}
