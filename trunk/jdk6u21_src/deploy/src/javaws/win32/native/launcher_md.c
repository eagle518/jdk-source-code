/*
 * @(#)launcher_md.c	1.19 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "system.h"
#include "jawsversion.h"

#define EULA_subkey "Software\\JavaSoft\\Java Runtime Environment\\"
#define EULA_value  "EULA"

typedef BOOL (*EULAPROC)(HWND hwnd);


#ifndef WIN_CONSOLE
extern int main(int argc, char** argv);

 /*
 *  Windows main-method
 */

int WINAPI WinMain(HINSTANCE inst, HINSTANCE previnst, LPSTR cmdline, int cmdshow) {
  return main(__argc, __argv);
}
#endif

/* Add GetApplicationHome to path. This is done instead of a bat file.
 *
 */
void LauncherSetup_md(char **argv) {
    char *path = getenv("PATH");
    char *home = sysGetApplicationHome();    
    char *newPath;
    TCHAR tmpname[MAX_PATH], jarname[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    if (path != NULL) {
        newPath = (char *)malloc(sizeof(char) * (strlen(path) +
                                                 strlen(home) + 12));
        sprintf(newPath, "PATH=%s;\"%s\"", path, home);
    }
    else {
        newPath = (char *)malloc(sizeof(char) * (strlen(home) + 10));
        sprintf(newPath, "PATH=\"%s\"", home);
    }
    putenv(newPath);

}
/* return 1 if EULA pass or EULA disabled, otherwise 0 (failed) */
int EULA_md(int argc, char** argv, int isPlayer) {

  int ret = 0;
  char eula_cmd[MAXPATHLEN];
  char eula_subkey_withversion[MAXPATHLEN];

  int eula_popup_enabled = 0;
  int i;

  /* no need to check EULA for javaws application manager */
  /* two ways to launch application manager               */
  /* "javaws" or "javaws -Xnosplash"                      */
  if (isPlayer || argc == 1 || (argc == 2 && !strcmp(argv[1], "-Xnosplash"))) {
    return 1;
  } else {
    /* no need to check EULA for -uninstall or -updateVersions option */    
    for (i = 0; i < argc; i++) {
      if (!strcmp(argv[i], "-uninstall") || 
	  !strcmp(argv[i], "-updateVersions")) {
	return 1;
      }
    }    
  }    

  /* add the JRE/JDK version to the EULA key */
  sprintf(eula_subkey_withversion,"%s%s",EULA_subkey,JAVAWS_JDK_VERSION);

  /* 
     check 
     HKEY_LOCAL_MACHINE\Software\JavaSoft\Java Runtime Environment
     \<unique-version>\EULA value:

     0 -> EULA dialog is disabled, default
     1 -> EULA dialog is enabled

     If HKLM EULA registry key value does not exist, EULA popup will be 
     disabled
  */
  eula_popup_enabled = sysGetRegistryValue(HKEY_LOCAL_MACHINE, eula_subkey_withversion, EULA_value);
  
  if (eula_popup_enabled == 1) {
    /* EULA popup enabled */

    /* 
       check 
       HKEY_CUREENT_USER\Software\JavaSoft\Java Runtime Environment
       \[ver]\EULA value:
       
       0 -> EULA is not accepted, default
       1 -> EULA is accepted
       
       If EULA registry key value does not exist, EULA will pop up
    */
    ret = sysGetRegistryValue(HKEY_CURRENT_USER, eula_subkey_withversion, EULA_value);
    
    if (ret != 1) {
      /* not accepted, popup EULA! */    
      ret = popEULA();
    } else {
      /* EULA accepted - continue launch */
      ret = 1;
    }
    
  } else {
    /* EULA popup disabled */
    ret = 1;
  }
  
  return ret;
}

/* using eula.dll to call eula dialog */
int popEULA() {

  HINSTANCE hinstLib;
  EULAPROC EulaAdd;
  int ret = 0;
  char eulaPath[MAX_PATH];

  /* eula.dll is located at JAVA_HOME/bin/eula.dll */
  sprintf(eulaPath, "%s%c%s%c%s", sysGetInstallJREHome(), FILE_SEPARATOR,
	  "bin", FILE_SEPARATOR, "eula.dll");

  hinstLib = LoadLibrary(eulaPath);

  if (hinstLib != NULL) {
    
    EulaAdd = (EULAPROC) GetProcAddress(hinstLib, "ShowEulaDialog");

    if (EulaAdd != NULL) {
      /* call the function */
      ret = (EulaAdd)(NULL);
    }
    
    FreeLibrary(hinstLib);
  }

  return ret;
}
