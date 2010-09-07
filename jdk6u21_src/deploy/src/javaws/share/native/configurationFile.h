/*
 * @(#)configurationFile.h	1.32 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef CONFIGURATION_FILE_H
#define CONFIGURATION_FILE_H

/* Configration file location and filename */
#define CFG_FILENAME     "deployment.properties"

/* Java Web Start configuration file entries */
#define CFG_JRE_KEY      "deployment.javaws.jre."
#define PLATFORM_VERSION ".platform"
#define PRODUCT_VERSION  ".product"
#define PRODUCT_LOCATION ".location"
#define INSTALL_PATH     ".path"
#define OS_NAME          ".osname"
#define OS_ARCH          ".osarch"
#define ISENABLED        ".enabled"
#define ISREGISTERED     ".registered"
#define CFG_SPLASH_MODE  "deployment.javaws.showSplashScreen"
#define CFG_SPLASH_CACHE "deployment.javaws.splash.index"
#define CFG_SECURE_PROPS "deployment.javaws.secure.properties"

#define JRELOCATORCLASSNAME "com.sun.deploy.panel.JreLocator"
#define PRODUCT_ID          "productVersion="
#define PLATFORM_ID         "platformVersion="
#define DEFHREF             "http://java.sun.com/products/autodl/j2se"

#define MAX_KEY_SIZE    50

#include "propertyParser.h"

typedef struct {
  char *platform_version;
  char *product_version;
  char *href;
  char *path;
  char *osname;
  char *osarch;
} JREDescription;

typedef struct {
  int index;
  int confirmed;
} JREIndexMap;

/*
 * Reads and interprets the configuration file
 */

int   isJRERegistered(int i);
int   isJREConfirmed(int i);
int   JreMatch(char* version, char* location, int index, int verbose);
int   matchVersionString(char* version, char* location, int verbose);
void  StoreConfigurationFile(int verbose);
void  SetProperty(char *key, char *value);
int   RescanJREs(int verbose);
int   LoadConfigurationFile(int verbose);
void  LoadCfgFile(char*, PropertyFileEntry**, int verbose);
int   DetermineVersion(char*version, char* location, int verbose);
int   GetDefaultJRE();
char* GetJREKey(int i, char* part);
char* GetJREPlatformVersion(int i);
char* GetJREProductVersion(int i);
char* GetJRELocation(int i);
char* GetJREJavaCmd(int i);
char* GetJREOsName(int i);
char* GetJREOsArch(int i);
int   isSplashScreenEnabled(void);
char* getDefaultJREs(void);
char* getSplashFile(char* url);
char *getConfigSecureProperties();
void SetJREKey(int i, char *part, char *value);
void SetJREPlatformVersion(int i, char *value);
void SetJREProductVersion(int i, char *value);
void SetJRELocation(int i, char *value);
void SetJREJavaCmd(int i, char *value);
void SetJREOsName(int i, char *value);
void SetJREOsArch(int i, char *value);
void SetJREEnabled(int i, char *value);
void SetJRERegistered(int i, char *value);
char* GetJREJavaDebugCmd(int i);
char* GetJREJavaCmd(int i);
void addToIndexArray(int newIndex);
/*
 * in launcher.c
 */
char * GetDeployJarPath(void);


#endif
