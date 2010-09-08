/*
 * @(#)linker_md.c	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Maintains a list of currently loaded DLLs (Dynamic Link Libraries)
 * and their associated handles. Library names are case-insensitive.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
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
        int n = (int)strlen(s);
        if (n >= len) n = len - 1;
        strncpy(buf, s, n);
        buf[n] = '\0';
        return n;
    }

    return 0;
}

/*
 * splits a path, based on its separator, the number of
 * elements is returned back in n. 
 * It is the callers responsibility to:
 *   a> check the value of n, n may be 0
 *   b> ignore any empty path elements
 *   c> free up the data.
 */
static char** split_path(const char* path, int* n) {
    char* inpath;
    char** opath;
    char* p;
    int count = 1;
    int i;
    
    *n = 0;
    if (path == NULL || strlen(path) == 0) {
        return NULL;
    }
    inpath = strdup(path);
    if (inpath == NULL) {
        return NULL;
    }
    p = strchr(inpath, PATH_SEPARATOR_CHAR);
    // get a count of elements to allocate memory
    while (p != NULL) {
        count++;
        p++;
        p = strchr(p, PATH_SEPARATOR_CHAR);
    }
    opath = (char**) calloc(count, sizeof(char*));
    if (opath == NULL) {
        return NULL;
    }

    // do the actual splitting
    p = inpath;
    for (i = 0 ; i < count ; i++) {
        size_t len = strcspn(p, PATH_SEPARATOR);
	// allocate the string and add terminator storage
        char* s = (char*)malloc((len + 1)*sizeof(char));
        if (s == NULL) {
            return NULL;
        }
        strncpy(s, p, len);
        s[len] = '\0';
        opath[i] = s;
        p += len + 1;
    }
    free(inpath);
    *n = count;
    return opath;
}

/*
 * Build a machine dependent library name out of a path and file name.
 */
void
dbgsysBuildLibName(char *holder, int holderlen, char *pname, char *fname)
{
    int n;
    int i;
    char c;
    char **pelements;
    const int pnamelen = pname ? (int)strlen(pname) : 0;
    c = (pnamelen > 0) ? pname[pnamelen-1] : 0;

    /* Quietly truncates on buffer overflow. Should be an error. */
    if (pnamelen + (int)strlen(fname) + 10 > holderlen) {
        *holder = '\0';
        return;
    }

    if (pnamelen == 0) {
        sprintf(holder, "%s.dll", fname);
    } else if (c == ':' || c == '\\') {
        sprintf(holder, "%s%s.dll", pname, fname);
    } else if (strchr(pname, PATH_SEPARATOR_CHAR) != NULL) {
        pelements = split_path(pname, &n);
        for (i = 0 ; i < n ; i++) {
            char* path = pelements[i];
            char lastchar;
            // really shouldn't be NULL but what the heck, check can't hurt
            size_t plen = (path == NULL) ? 0 : strlen(path);
            if (plen == 0) {
                continue; // skip empty path values
            }
            lastchar = path[plen - 1];
            if (lastchar == ':' || lastchar == '\\') {
                sprintf(holder, "%s%s.dll", path, fname);
            } else {
                sprintf(holder, "%s\\%s.dll", path, fname);
            }
            if (GetFileAttributes(holder) != INVALID_FILE_ATTRIBUTES) {
                break;
            }
        }
        // release the storage
        for (i = 0 ; i < n ; i++) {
           if (pelements[i] != NULL) {
              free(pelements[i]);
           }
        }
        if (pelements != NULL) {
           free(pelements);
        }
    } else {
        sprintf(holder, "%s\\%s.dll", pname, fname);
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
