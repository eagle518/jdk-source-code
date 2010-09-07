/*
 * @(#)system_md.h	1.20 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef SYSTEM_MD_H
#define SYSTEM_MD_H
/* Defines a common interface to a set of OS specific methods 
*/

#define PLATFORM "Windows"

/*
 *  Windows specific includes
 */
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <shlobj.h>
#include <tchar.h>

#define BREAKPOINT   __asm { int 3 }

#define FILE_SEPARATOR '\\'
#define PATH_SEPARATOR ';'

#define JRE_VERSION_REG_KEY "SOFTWARE\\JavaSoft\\Java Runtime Environment"
#define JAVAHOMEVALUE       "JavaHome"

typedef WORD twchar_t;

#endif
