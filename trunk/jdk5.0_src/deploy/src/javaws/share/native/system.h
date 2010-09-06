/*
 * @(#)system.h	1.26 04/01/09
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef SYSTEM_H
#define SYSTEM_H
/* Defines a common interface to a set of OS specific methods \
*/

#ifndef MAXPATHLEN 
#define MAXPATHLEN 1024
#endif

/* Common includes */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
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
char*  sysQuoteString         (char* s);
char*  sysGetSplashExtension  (void);
char*  sysGetDeploymentSystemHome (void);
char*  sysGetDeploymentUserHome   (void);

int    sysExec                (int kind, char* path, char* argv[]);

char*  sysGetApplicationHome  (void);
char*  sysGetInstallJRE       (void);
char*  sysGetJavawsbin        (void);

char*  sysTempnam             (void);

int    sysStrCaseCmp          (char* s1, char* s2);
char*  sysGetLocaleStr        (void);
char*  sysWideCharToMBCS      (twchar_t *, size_t);
char*  sysMBCSToSeqUnicode    (char *);

char*  sysGetOsName           (void);
char*  sysGetOsArch           (void);

int    sysSplash              (int port, char *file, char *file2);
void   sysGetRegistryJREs     (JREDescription [], int *);
void   sysExec2Buf            (char *, int argc, char *[], char *, int *);
void   sysCreateDirectory     (char *);
void   sysExit                (int copiedfile, int copiedFileIndex, char **argv);
char*  sysSaveConvert         (unsigned short *theString);
int  sysFindSiFile          (char *canonicalHome, char *siFilename);

void   sysSetStartTime          (void);
void   sysSetEndTime            (void);
void   sysPrintTimeUsed         (char *filename);
char*  sysGetDebugJavaCmd       (char *javaCmd);
#endif
