/*
 * @(#)j2secmod_md.h	1.4 10/04/08
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// in nss.h:
// extern PRBool NSS_VersionCheck(const char *importedVersion);
// extern SECStatus NSS_Init(const char *configdir);
typedef int (*FPTR_VersionCheck)(const char *importedVersion);
typedef int (*FPTR_Init)(const char *configdir);

// in secmod.h
//extern SECMODModule *SECMOD_LoadModule(char *moduleSpec,SECMODModule *parent,
//							PRBool recurse);
//char **SECMOD_GetModuleSpecList(SECMODModule *module);
//extern SECMODModuleList *SECMOD_GetDBModuleList(void);

typedef void *(*FPTR_LoadModule)(char *moduleSpec, void *parent, int recurse);
typedef char **(*FPTR_GetModuleSpecList)(void *module);
typedef void *(*FPTR_GetDBModuleList)(void);

