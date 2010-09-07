/*
 *  @(#)GetLibrary.cpp	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * GetLibrary.cpp by X.Lu
 * These functions create, initialize, and shutdown a plugin.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <libintl.h>
#include <dlfcn.h>
#include "JDSupportUtils.h"
#include "JDCOMUtils.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

static char* FindPluginDir(void) {
    Dl_info dlinfo;
    static int dummy;
    char buf[MAXPATHLEN];
    char real[MAXPATHLEN];

    /* Now: Only use the JRE which came with the plugin.

     Formerly:
       Use the NPX_PLUGIN_PATH to find the directories holding plugins
       Find the first of those that has a ../java directory.
    */
    dladdr((void *)&dummy, &dlinfo);
    strcpy(buf, dlinfo.dli_fname);
   
    if (realpath(buf, real) == NULL) {
	fprintf(stderr, "Error: realpath(`%s') failed.\n", buf);
	return NULL;
    }

    *(strrchr(real, '/')) = '\0';  /* executable file      */
   
    return strdup(real);
}

JDresult LoadNSCore(void* *libjpinsp)
{
   char libpath[MAXPATHLEN];
   char* pluginloc = FindPluginDir();
   JDresult err = JD_ERROR_FAILURE;

   if (pluginloc == NULL) {
       fprintf(stderr, "Can not determin plugin path!\n");
       return err;
   }
   
   char coreName[MAXPATHLEN]; 
   /* On Linux, if the link path  contains gcc29/ns4, always use
      libjavaplugin_nscp_gcc29.
   */
#ifdef __linux__
   if (strstr((const char*)pluginloc, "gcc29") != NULL ||
       strstr((const char*)pluginloc, "ns4") != NULL)
     strcpy(coreName, "libjavaplugin_nscp_gcc29");
   else
     strcpy(coreName, "libjavaplugin_nscp");
#else
   strcpy(coreName, "libjavaplugin_nscp");
#endif

    *(strrchr(pluginloc, '/')) = '\0';  /* browser directory    */
    *(strrchr(pluginloc, '/')) = '\0';  /* LIBARCH directory    */
    *(strrchr(pluginloc, '/')) = '\0';  /* plugin directory     */

   snprintf(libpath, sizeof libpath, "%s/lib/%s/%s.so", pluginloc, LIBARCH, coreName);

   *libjpinsp = dlopen(libpath, RTLD_LAZY + RTLD_GLOBAL);

   if (*libjpinsp == NULL) {
     fprintf(stderr, dlerror());
     err = JD_ERROR_FAILURE;
   }
   else
     err = JD_OK;

    free(pluginloc);

    return err;
}

void UnloadNSCore(void* libjpinsp) {
  
  dlclose(libjpinsp);
 
  return;
}


