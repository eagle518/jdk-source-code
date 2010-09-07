/*
 * @(#)system.h	1.28 05/02/28
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef SYSTEM_H
#define SYSTEM_H
/* Defines a common interface to a set of OS specific methods \
*/

#ifndef MAXPATHLEN 
#define MAXPATHLEN 1024
#endif
/**
 * Microsoft Windows XP or later: 8191 characters
 * Microsoft Windows 2000 or Windows NT 4.0: 2047 character
 */
#define CMDLNMAXLEN  2047

/* Common includes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <wchar.h>
#include "msgString.h" 
/* Include platform-specific defines */
#include "system_md.h"
#include "configurationFile.h"

/* The sysExec can either fork a new process, replace the current
 * process, or fork exec and wait for child to finish.
 */
#define SYS_EXEC_FORK      0
#define SYS_EXEC_REPLACE   1
/* This is treated as SYS_EXEC_FORK on Unix. */
#define SYS_EXEC_WAIT      2

/* Max String length in property file */
#define MAXSTRINGLEN 2048

/*
 *  Platform indpendent UNIX socket/io layer, etc.
 */
void   sysErrorExit           (char* msg);
void   sysMessage             (char* msg);

void   sysInitSocketLibrary   (void);
void   sysCloseSocket         (SOCKET s);
SOCKET sysCreateListenerSocket(int* port);
SOCKET sysCreateServerSocket  (int  port);
SOCKET sysCreateClientSocket  (int  port);
SOCKET sysTestServerSocketCreatable(int* port);
int    sysWriteSocket         (SOCKET s, char* str);
char*  sysReadSocket          (SOCKET s);
char*  sysQuoteString         (char *s);
char*  sysGetSplashExtension  (void);
char*  sysGetDeploymentSystemHome (void);
char*  sysGetDeploymentUserHome   (void);

void   sysSetupQuotesWholePropertySpec(int verbose);
int    sysGetQuotesWholePropertySpec(void);

int    sysExec                (int kind, char* path, char* argv[]);

char*  sysGetApplicationHome  (void);
char*  sysGetInstallJRE       (void);
char*  sysGetJavawsbin        (void);
char*  sysGetJavabin         (void);

char*  sysTempnam             (void);

/**
 * @return If the output was truncated due to the limit, then the return value is the number of characters (without EOS),
 *         which would have been written to the final string if, enough space had been available.
 *         Thus, a return value of size or more means that the output was truncated.
 *         If an output error is encountered, a negative value is returned.
 */
int    sysStrNPrintF          (char* str, size_t size, const char *format, ...);

int    sysStrCaseCmp          (char* s1, char* s2);
int    sysStrNCaseCmp         (char* s1, char* s2, size_t n);
char*  sysGetLocaleStr        (void);
char*  sysWideCharToMBCS      (twchar_t *, size_t);
char*  sysMBCSToSeqUnicode    (char *);

char*  sysGetOsName           (void);
char*  sysGetOsArch           (void);

int    sysSplash              (int port, char *file);
void   sysGetRegistryJREs     (JREDescription [], int *);
void   sysExec2Buf            (char *, int argc, char *[], char *, int *);
void   sysCreateDirectory     (char *);
void   sysExit                (char *copyfilename);
char*  sysSaveConvert         (unsigned short *theString);
int  sysFindSiFile          (char *canonicalHome, char *siFilename);
int  sysFindSiPort            (char *canonicalHome);
char*  sysFindSiNumber        (char *canonicalHome);
char*  sysGetSiFilePath       (char *siFilename);

int   sysSetStartTime          (void);
void   sysSetEndTime            (void);
void   sysPrintTimeUsed         (char *filename);
char*  sysGetDebugJavaCmd       (char *javaCmd);
char*  sysGetJarLib             (void);
char*  sysGetSecurityLib        (void);
char*  sysGetLibPath            (char *filename);

void   recursive_create_directory(char *path);
char* sysGetJavawsResourcesLib(void);

#endif
