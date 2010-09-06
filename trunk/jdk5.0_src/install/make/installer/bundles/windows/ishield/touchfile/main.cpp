/*
 * @(#)main.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
#include "windows.h"
#include <stdio.h>


// SetFileToArbitaryTime - sets last write time to arbitary system time
// Return value - TRUE if successful, FALSE otherwise
// hFile  - must be a valid file handle

BOOL SetFileToArbitaryTime(HANDLE hFile)
{
    FILETIME ft;
    SYSTEMTIME st;
    BOOL f;

    st.wYear = 2003;
    st.wMonth = 1;
    st.wDayOfWeek = 2;
    st.wDay = 1;
    st.wHour = 16;
    st.wMinute = 0;
    st.wSecond = 0;
    st.wMilliseconds = 0;

    SystemTimeToFileTime(&st, &ft);  // converts to file time format
    f = SetFileTime(hFile,           // sets last-write time for file
        (LPFILETIME) &ft, (LPFILETIME) NULL, (LPFILETIME) NULL);

    return f;
}


// 
// Set the creation date on all files in __argv[1] to an arbitary date.
//
// This is necessary for supporting base image patching because
// InstallShield only generates the same CAB file from the file set if
// the file content as well as the creation date are the same.
//
int main()
{
    if (__argc != 2)
    {
	fprintf(stderr, "Usage: %s directory\n", __argv[0]);
	return 1;
    }

    char szDirectory[1024];
    WIN32_FIND_DATA findData;

    if (__argv[1][strlen(__argv[1]) - 1] != '\\')
	wsprintf(szDirectory, "%s\\*", __argv[1]);
    else
	wsprintf(szDirectory, "%s*", __argv[1]);

    ::ZeroMemory(&findData, sizeof(WIN32_FIND_DATA));
    
    HANDLE hFind = FindFirstFile(szDirectory, &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
	do
	{
	    // skip "." and ".."
	    if (stricmp(findData.cFileName, ".") == 0 
		|| stricmp(findData.cFileName, "..") == 0)
		continue;

	    // Determine full file path
	    char szFile[1024];

	    if (__argv[1][strlen(__argv[1]) - 1] != '\\')
		wsprintf(szFile, "%s\\%s", __argv[1], findData.cFileName);
	    else
		wsprintf(szFile, "%s%s", __argv[1], findData.cFileName);


	    printf("Changing creation date of %s ...\n", szFile);

	    HANDLE hFile = CreateFile(szFile, 
		                      GENERIC_WRITE,             // open for writing 
				      FILE_SHARE_WRITE,          // share for writing 
				      NULL,                      // no security 
				      OPEN_EXISTING,             // existing file only 
				      FILE_ATTRIBUTE_NORMAL,     // normal file 
				      NULL);                     // no attr. template 

	    if (hFile != INVALID_HANDLE_VALUE) 
	    { 
		if (SetFileToArbitaryTime(hFile))
		    printf("Creation date of %s is changed to 01/01/2003 8:00am PST\n", szFile);
		else
		    printf("Failure: Cannot change creation date of %s to 01/01/2003 8:00am PST\n", szFile);

		CloseHandle(hFile);
	    }
	    else
	    {
		printf("Failure: Cannot open %s to change creation date\n", szFile);
	    }
	}
	while (FindNextFile(hFind, &findData));

	if (hFind != NULL)
	    FindClose(hFind);
    }

    return 0;
}

