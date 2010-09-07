/*
 * @(#)utils_md.cpp	1.3 10/04/16
 *
 * Copyright (c) 2009, 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "utils.h"
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

int isValidUrl(const char* jnlp_url, const char* docbase_url) {
    // for now - just return 0 (false) to disable launchJNLP
    return 0;
}

static int GetDirectoryForModuleContainingAddress(void* address,
                                                  char* directoryContainingModule);

// invoke javaws with -docbase <docbase> <jnlp>
int launchJNLP(const char* jnlp_url, const char* docbase_url) {
   char jvmPath[PATH_MAX + 1];
   char javaHomeLibPath[PATH_MAX + 1];
   char javawsPath[PATH_MAX + 1];
   char* lastSlash;

   // validate that jnlp_url and docbase_url constitute a valid url
   if (!isValidUrl(jnlp_url, docbase_url)) {
      return 0;
   }

   // Instead of consulting deployment.properties,
   // we can figure out which JRE we live in by finding out the
   // path name to the loaded module (DSO) we are in right now.
   if (!GetDirectoryForModuleContainingAddress((void*) &launchJNLP, jvmPath)) {
      // Should not happen
      return 0;
   }
   // Copy this off for later manipulation
   strcpy(javaHomeLibPath, jvmPath);
   // Note that javaHomeLibPath doesn't yet contain the real Java
   // home directory; it ends with the java_home/lib/$arch (e.g., "i386")

   // Go up two slash to get to java home
   lastSlash = strrchr(javaHomeLibPath, '/');
 
   if (lastSlash == NULL) {
      // Should not happen
      return 0;
   }
   *lastSlash = '\0';

   lastSlash = strrchr(javaHomeLibPath, '/');
   
   if (lastSlash == NULL) {
      // Should not happen   
      return 0;
   }
   *lastSlash = '\0';
  
   // append /bin/javaws to java_home
   snprintf(javawsPath, sizeof(javawsPath), "%s/bin/javaws", &javaHomeLibPath);
 
   // construct args for javaws process
   char* argv[5];
   argv[0] = (char*)malloc(sizeof(char) * strlen(javawsPath) + 1);
   strcpy(argv[0], javawsPath);

   argv[1] = (char*)malloc(sizeof(char) * strlen("-docbase") + 1);
   strcpy(argv[1], "-docbase");

   argv[2] = (char*)malloc(sizeof(char) * strlen(docbase_url) + 1);
   strcpy(argv[2], docbase_url);

   argv[3] = (char*)malloc(sizeof(char) * strlen(jnlp_url) + 1);
   strcpy(argv[3], jnlp_url);

   argv[4] = NULL;

   int pid;

   // launch javaws process
   // based on sysExec on src/javaws/solaris/native/system_md.c
   if ((pid = fork()) == 0) {
      int err = execv(javawsPath, argv);
      /* It's neccessary to call "_exit" here, rather than exit, see
       * the fork(2) manual page.
       */
      _exit(-1);
   } else {
      return 0;
   }

   return 1;
}

static int GetDirectoryForModuleContainingAddress(void* addr,
                                                  char* directoryContainingModule)
{
    // Attempts to find the full path to the shared object (.so) which
    // contains the given address. There is no trailing slash ('/') on
    // the returned path. Returns 1 if successful, 0 if not.
    // directoryContainingModule may be mutated regardless.
    Dl_info info;
    int res;
    char* lastSlash = NULL;

    res = dladdr(addr, &info);
    if (res == 0)
        return 0;

    // bounds check
    if (strlen(info.dli_fname) >= PATH_MAX) {
      return 0;
    }

    strcpy(directoryContainingModule, info.dli_fname);
    lastSlash = strrchr(directoryContainingModule, '/');
    if (lastSlash == NULL)
        return 0;
    *lastSlash = '\0';
    return 1;
}
