/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
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
#include <mbstring.h>
#include "UserProfile.h"
#include "WrapperUtils.h"



void GetSingleMSIFileNames(LPTSTR lpszLocalFileName, BOOL b64bit)
{
    GetUserShellFolder(lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%s\\Sun\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%sJava\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    if (b64bit) {
      wsprintf(lpszLocalFileName, "%s%s%s_x64", lpszLocalFileName, BUNDLE, VERSION);
    }
    else {
      wsprintf(lpszLocalFileName, "%s%s%s", lpszLocalFileName, BUNDLE, VERSION);
    }
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%s\\%s%s.msi", lpszLocalFileName, BUNDLE, VERSION);

}
