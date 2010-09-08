/*
 * @(#)j2secmod_md.h	1.4 10/04/08
 *
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>

// in nss.h:
// extern PRBool NSS_VersionCheck(const char *importedVersion);
// extern SECStatus NSS_Init(const char *configdir);
typedef int __declspec(dllimport) (*FPTR_VersionCheck)(const char *importedVersion);
typedef int __declspec(dllimport) (*FPTR_Init)(const char *configdir);

// in secmod.h
//extern SECMODModule *SECMOD_LoadModule(char *moduleSpec,SECMODModule *parent,
//							PRBool recurse);
//char **SECMOD_GetModuleSpecList(SECMODModule *module);
//extern SECMODModuleList *SECMOD_GetDBModuleList(void);

typedef void __declspec(dllimport) *(*FPTR_LoadModule)(char *moduleSpec, void *parent, int recurse);
typedef char __declspec(dllimport) **(*FPTR_GetModuleSpecList)(void *module);
typedef void __declspec(dllimport) *(*FPTR_GetDBModuleList)(void);

