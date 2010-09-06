/*
 * @(#)sqlunx.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************
** SQLUNX.H - Mappings of Windows-style declarations and typedefs
**	      for unix.
**
** Copyright: 1992-1998 INTERSOLV, Inc.	
** This software contains confidential and proprietary
** information of INTERSOLV, Inc.
*********************************************************************/

#ifndef __SQLUNX
#define __SQLUNX

/* Unix versions of Wintel declaration modifiers */

#define NEAR
#define FAR
#define EXPORT
#define PASCAL
#define VOID void
#define CALLBACK
#define _cdecl
#define __stdcall

/* Windows-style typedefs */

typedef VOID * HANDLE;
typedef unsigned short WORD;
typedef unsigned int UINT;
#ifndef _LP64
typedef unsigned long DWORD;
#else
typedef unsigned int DWORD;
#endif
typedef unsigned char BYTE;
#ifndef _LP64
typedef long LONG;
#else
typedef int LONG;
#endif
typedef int BOOL;
typedef VOID * LPVOID;
typedef VOID * PVOID;
typedef VOID * HMODULE;
typedef int GLOBALHANDLE;
typedef int (*FARPROC)();
typedef char *LPSTR;
typedef const char * LPCSTR;
typedef VOID * HINSTANCE;
typedef VOID * HWND;
typedef unsigned int WPARAM;
#ifndef _LP64
typedef unsigned long LPARAM;
#else
typedef unsigned int LPARAM;
#endif
typedef VOID * HKEY; 
typedef VOID * PHKEY;
typedef BYTE * LPBYTE;
typedef char CHAR;
typedef BOOL * LPBOOL;
typedef DWORD * LPDWORD;
typedef char * LPWSTR;
typedef const char * LPCWSTR;
typedef char TCHAR;
typedef char WCHAR;
typedef char VCHAR;
typedef TCHAR * LPTSTR;
typedef const TCHAR* LPCTSTR;

#endif
