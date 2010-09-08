/*
 * @(#)linker_md.c	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Adapted from JDK 1.2 linker_md.c v1.37. Note that we #define
 * NATIVE here, whether or not we're running solaris native threads.
 * Outside the VM, it's unclear how we can do the locking that is
 * done in the green threads version of the code below. 
 */
#define NATIVE
                         
/*
 * Machine Dependent implementation of the dynamic linking support
 * for java.  This routine is Solaris specific.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "path_md.h"
#ifndef NATIVE
#include "iomgr.h"
#include "threads_md.h"
#endif

/*
 * create a string for the JNI native function name by adding the
 * appropriate decorations.
 */
int
dbgsysBuildFunName(char *name, int nameLen, int args_size, int encodingIndex)
{
  /* On Solaris, there is only one encoding method. */
    if (encodingIndex == 0)
        return 1;
    return 0;
}

/*
 * splits a path, based on its separator, the number of
 * elements is returned back in n. 
 * It is the callers responsibility to:
 *   a> check the value of n, and n may be 0
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
        char* s  = (char*)malloc((len + 1)*sizeof(char));
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
 * create a string for the dynamic lib open call by adding the
 * appropriate pre and extensions to a filename and the path
 */
void
dbgsysBuildLibName(char *holder, int holderlen, char *pname, char *fname)
{
    int n;
    int i;
    char** pelements;
    struct stat statbuf;
    const int pnamelen = pname ? strlen(pname) : 0;

    /* Quietly truncate on buffer overflow.  Should be an error. */
    if (pnamelen + (int)strlen(fname) + 10 > holderlen) {
        *holder = '\0';
        return;
    }

    if (pnamelen == 0) {
        (void)snprintf(holder, holderlen, "lib%s.so", fname);
    } else if (strchr(pname, PATH_SEPARATOR_CHAR) != NULL) {
      pelements = split_path(pname, &n);
      for (i = 0 ; i < n ; i++) {
          // really shouldn't be NULL but what the heck, check can't hurt
	  if (pelements[i] == NULL || strlen(pelements[i]) == 0) {
	      continue; // skip the empty path values
	  }
	  snprintf(holder, holderlen, "%s/lib%s.so", pelements[i], fname);
	  if (stat(holder, &statbuf) == 0) {
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
        (void)snprintf(holder, holderlen, "%s/lib%s.so", pname, fname);
    }
}

#ifndef NATIVE
extern int thr_main(void);
#endif

void *
dbgsysLoadLibrary(const char *name, char *err_buf, int err_buflen)
{
    void * result;
#ifdef NATIVE
    result = dlopen(name, RTLD_LAZY);
#else
    sysMonitorEnter(greenThreadSelf(), &_dl_lock);
    result = dlopen(name, RTLD_NOW);
    sysMonitorExit(greenThreadSelf(), &_dl_lock);
    /*
     * This is a bit of bulletproofing to catch the commonly occurring
     * problem of people loading a library which depends on libthread into
     * the VM.  thr_main() should always return -1 which means that libthread
     * isn't loaded.
     */
    if (thr_main() != -1) {
         VM_CALL(panic)("libthread loaded into green threads");
    }
#endif
    if (result == NULL) {
        (void)strncpy(err_buf, dlerror(), err_buflen-2);
        err_buf[err_buflen-1] = '\0';
    }
    return result;
}

void dbgsysUnloadLibrary(void *handle)
{
#ifndef NATIVE
    sysMonitorEnter(greenThreadSelf(), &_dl_lock);
#endif
    (void)dlclose(handle);
#ifndef NATIVE
    sysMonitorExit(greenThreadSelf(), &_dl_lock);
#endif
}

void * dbgsysFindLibraryEntry(void *handle, const char *name)
{
    void * sym;
#ifndef NATIVE
    sysMonitorEnter(greenThreadSelf(), &_dl_lock);
#endif
    sym =  dlsym(handle, name);
#ifndef NATIVE
    sysMonitorExit(greenThreadSelf(), &_dl_lock);
#endif
    return sym;
}
