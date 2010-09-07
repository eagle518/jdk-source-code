/*
 * @(#)linker_md.c	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Maintains a list of currently loaded DLLs (Dynamic Link Libraries)
 * and their associated handles. Library names are case-insensitive.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sys.h"

#include "path_md.h"

/*
 * From system_md.c v1.54
 */
int
dbgsysGetLastErrorString(char *buf, int len)
{
    long errval;

    if ((errval = GetLastError()) != 0) {
	/* DOS error */
	int n = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
			      NULL, errval,
			      0, buf, len, NULL);
	if (n > 3) {
	    /* Drop final '.', CR, LF */
	    if (buf[n - 1] == '\n') n--;
	    if (buf[n - 1] == '\r') n--;
	    if (buf[n - 1] == '.') n--;
	    buf[n] = '\0';
	}
	return n;
    }

    if (errno != 0) {
	/* C runtime error that has no corresponding DOS error code */
	const char *s = strerror(errno);
	int n = strlen(s);
	if (n >= len) n = len - 1;
	strncpy(buf, s, n);
	buf[n] = '\0';
	return n;
    }

    return 0;
}

/* 
 * Rest Adapted from JDK1.2 linker_md.c	1.43 98/09/28
 */
 
/*
 * create a string for the JNI native function name by adding the
 * appropriate decorations.
 *
 * On Win32, "__stdcall" functions are exported differently, depending
 * on the compiler. In MSVC 4.0, they are decorated with a "_" in the 
 * beginning, and @nnn in the end, where nnn is the number of bytes in 
 * the arguments (in decimal). Borland C++ exports undecorated names.
 *
 * dbgsysBuildFunName handles different encodings depending on the value 
 * of encodingIndex. It returns 0 when handed an out-of-range
 * encodingIndex.
 */
int
dbgsysBuildFunName(char *name, int nameMax, int args_size, int encodingIndex)
{
  if (encodingIndex == 0) {
    /* For Microsoft MSVC 4.0 */
    char suffix[6];    /* This is enough since Java never has more than 
			   256 words of arguments. */
    int nameLen;
    int i;

    sprintf(suffix, "@%d", args_size * 4);
    
    nameLen = strlen(name);
    if (nameLen >= nameMax - 7)
        return 1;
    for(i = nameLen; i > 0; i--)
        name[i] = name[i-1];
    name[0] = '_';
    
    sprintf(name + nameLen + 1, "%s", suffix);
    return 1;
  } else if (encodingIndex == 1)
    /* For Borland, etc. */
    return 1;
  else
    return 0;
}

/*
 * Build a machine dependent library name out of a path and file name.
 */
void
dbgsysBuildLibName(char *holder, int holderlen, char *pname, char *fname)
{
    const int pnamelen = pname ? strlen(pname) : 0;
    const char c = (pnamelen > 0) ? pname[pnamelen-1] : 0;
    char *suffix;

#ifdef DEBUG   
    suffix = "_g";
#else
    suffix = "";
#endif 

    /* Quietly truncates on buffer overflow. Should be an error. */
    if (pnamelen + strlen(fname) + 10 > (unsigned int)holderlen) {
        *holder = '\0';
        return;
    }

    if (pnamelen == 0) {
        sprintf(holder, "%s%s.dll", fname, suffix);
    } else if (c == ':' || c == '\\') {
        sprintf(holder, "%s%s%s.dll", pname, fname, suffix);
    } else {
        sprintf(holder, "%s\\%s%s.dll", pname, fname, suffix);
    }
}

void *
dbgsysLoadLibrary(const char * name, char *err_buf, int err_buflen)
{
    void *result = LoadLibrary(name);
    if (result == NULL) {
	/* Error message is pretty lame, try to make a better guess. */
	long errcode = GetLastError();
	if (errcode == ERROR_MOD_NOT_FOUND) {
	    strncpy(err_buf, "Can't find dependent libraries", err_buflen-2);
	    err_buf[err_buflen-1] = '\0';
	} else {
	    dbgsysGetLastErrorString(err_buf, err_buflen);
	}
    }
    return result;
}

void dbgsysUnloadLibrary(void *handle)
{
    FreeLibrary(handle);
}

void * dbgsysFindLibraryEntry(void *handle, const char *name)
{
    return GetProcAddress(handle, name);
}
