/*
 * @(#)launcher.h	1.21 04/04/03
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef LAUNCHER_H
#define LAUNCHER_H

#include "util.h"
#include "system.h"
#include "configurationFile.h"
#include "launchFile.h"
#include "splashFile.h"
#include "msgString.h"
#include "jpda.h"



/*
 * Declarations
 */

/* Launching of Java */
void  LaunchJava                  (int jreIndex, char* vmargs, int argc, char** argv, int replace, int wait, int copiedFile, char *splash1, char *splash2, char *initialHeap, char *maxHeap, char *perflog, char *launchTime, char** vmProps, int vmPropsCount, int silent  _JPDA_OPTIONS_T);
char *createArgsFile              (int, char **, int);
int   ConvertVmArgs               (char *vmoption, char** vmargs, char *initialHeap, char *maxHeap, int maxargs    _JPDA_OPTIONS_T);
char* GetBootClassPathArg         (void);
char* GetDeployJarPath            (void);
char* GetBootClassPath            (void);
char* GetNonBootClassPath         (void);
char* GetJnlpxHomeOption          (void);
char* GetJnlpxSplashPortOption    (void);
char* GetJnlpxJVMOption           (char* jdkkey);
char* GetJnlpxRemoveOption        (int copiedFile);
char* GetSecurityPolicyOption     (void);
char* GetTrustProxyOption         (void);
char* GetVerifierOption		  (void);
void  ShowSplashScreen            (char *splash1, char *splash2);
int   GetSplashPort               (void);
char *GetHeapSizeOption           (char *initialHeap, char *maxHeap);
char* GetSystemClassLoaderOption  (void);
char *GetVMArgsOption             (char *vmoptions);
char *GetHeadlessOption           (void);
char *GetPerfLogOption            (char *perflog);
char *GetLaunchTimeOption         (char *launchTime);
char *GetMozExtDir		  (char *mozExtDir);

/* Launchfile parsing */
void  ScanFileArgumentForOptions  (int argc, char** argv, char** jnlpfile, char** canonicalHome, char** jreVersion, char** jreLocation, char** vmargs, int *copiedfile, char** splash1, char **splash2, char **initialHeap, char **maxHeap    _JPDA_OPTIONS_T, int *wait, int *isPlayer, int *copiedFileIndex, int *silent, char** perflog, char** launchTime, char** vmProps, int *vmPropsCount);
char *ReadFileNameFromFile(char *file);


void  LauncherSetup_md            (char **argv);

int SetupSingleInstance(char *jnlpfile, char* canonicalHome, int argc, char** argv);

/* #define NO_SPLASH     NULL	//  CL: clean up (not used) */
/* #define APP_SPLASH    "1"	//  CL: clean up (not used) */
/* #define PLAYER_SPLASH "2"	//  CL: clean up (not used) */


#endif LAUNCHER_H
