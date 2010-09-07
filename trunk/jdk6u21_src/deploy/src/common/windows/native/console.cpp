/*
 * @(#)console.cpp	1.5 06/05/08
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <jni.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

extern "C" {


typedef void (JNICALL *JVM_DUMPALLSTACKS) (JNIEnv *env, jclass unused);

typedef int (*LPFN_DUP)(int);
typedef int (*LPFN_DUP2)(int, int);
typedef long (*LPFN_LSEEK)(int, long, int);
typedef int (*LPFN_OPEN)(const char*, int, int);
typedef int (*LPFN_FSTAT)(int, struct _stat*);
typedef int (*LPFN_CLOSE)(int);
typedef int (*LPFN_READ)(int, void*, unsigned int);

JNIEXPORT jobject JNICALL Java_com_sun_deploy_util_ConsoleHelper_preMustangDumpAllStacksImpl
(JNIEnv* env, jclass clazz)
{ 
  /* load the msvcrt.dll from VC6 */
  HMODULE hvc6Module = GetModuleHandle("msvcrt.dll");

  if (hvc6Module == NULL) {
     return NULL;
  }

  LPFN_OPEN pfnOpen = (LPFN_OPEN)GetProcAddress(hvc6Module, "_open");
  LPFN_DUP pfnDup = (LPFN_DUP)GetProcAddress(hvc6Module, "_dup");
  LPFN_DUP2 pfnDup2 = (LPFN_DUP2)GetProcAddress(hvc6Module, "_dup2");
  LPFN_LSEEK pfnLseek = (LPFN_LSEEK)GetProcAddress(hvc6Module, "_lseek");
  LPFN_FSTAT pfnFstat = (LPFN_FSTAT)GetProcAddress(hvc6Module, "_fstat");
  LPFN_READ pfnRead = (LPFN_READ)GetProcAddress(hvc6Module, "_read");
  LPFN_CLOSE pfnClose = (LPFN_CLOSE)GetProcAddress(hvc6Module, "_close");

  if (pfnOpen == NULL || pfnDup == NULL || pfnDup2 == NULL ||
     pfnLseek == NULL || pfnFstat == NULL || pfnRead == NULL || 
     pfnClose == NULL) {
     return NULL;
  }
  
  /* Open a tmp file to record thread info */ 
  char userTempPath[MAX_PATH];
  LPTSTR lpszUserInfo = userTempPath;
  GetTempPath(MAX_PATH, lpszUserInfo);
  char* tmpFile = _tempnam(lpszUserInfo, NULL); 

  int tfildes = (pfnOpen)(tmpFile, O_CREAT|O_RDWR, 0666);
    
  /* Duplicate a standard file descriptor */ 
  int sfildes = (pfnDup)(1);

  /* file descriptor 1 point to tfildes */ 
  (pfnDup2)(tfildes, 1);
   
  HMODULE hModule = GetModuleHandle("jvm.dll");

  if (hModule != NULL) {
     // Look up JVM_DumpAllStacks
     JVM_DUMPALLSTACKS pfnDumpAllStacks = (JVM_DUMPALLSTACKS) ::
       GetProcAddress(hModule, "_JVM_DumpAllStacks@8");
  
     // Dump stacks
     if (pfnDumpAllStacks) {
       pfnDumpAllStacks(env, NULL);    
     }
  }
 
  /* file descriptor 1 point back to sfildes */ 
  (pfnDup2)(sfildes, 1);

  /* Move the file pointer to the begining */ 
  (pfnLseek)(tfildes, 0, SEEK_SET);
  
  struct _stat buf;

  /* Get the file size */ 
  (pfnFstat)(tfildes, &buf);

  int len = buf.st_size; 
  
  char* pszConsoleOutput = NULL;

  /* Read the content in tmp file into output buffer */ 
  if (len > 0) { 
    pszConsoleOutput = (char *) calloc(len + 1, sizeof(char)); 

    (pfnRead)(tfildes, pszConsoleOutput, len);
  } 
  
  jstring str = NULL; 

  /* Display the thread output in Java Console */ 
  if (pszConsoleOutput != NULL) {
    str = env->NewStringUTF(pszConsoleOutput); 
  }

  /* Release memory */ 
  free(pszConsoleOutput);

  (pfnClose)(tfildes);

  return str;
}

JNIEXPORT jobject JNICALL Java_com_sun_deploy_util_ConsoleHelper_dumpAllStacksImpl
(JNIEnv* env, jclass clazz)
{   
  char userTempPath[MAX_PATH];
  jstring str = NULL; 
  char* pszConsoleOutput = NULL;
  char* tmpFile = NULL; 
  struct stat buf; 
  int sfildes, tfildes; 
  int len; 
  
  /* Open a tmp file to record thread info */ 
  LPTSTR lpszUserInfo = userTempPath;
  GetTempPath(MAX_PATH, lpszUserInfo);
  tmpFile = _tempnam(lpszUserInfo, NULL); 

  tfildes = open(tmpFile, O_CREAT|O_RDWR, 0666); 

  /* Duplicate a standard file descriptor */ 
  sfildes = _dup(1); 
  
  /* file descriptor 1 point to tfildes */ 
  _dup2(tfildes, 1); 

  HMODULE hModule = GetModuleHandle("jvm.dll");
  
  // Look up JVM_DumpAllStacks
  JVM_DUMPALLSTACKS pfnDumpAllStacks = (JVM_DUMPALLSTACKS) ::
    GetProcAddress(hModule, "_JVM_DumpAllStacks@8");
  
  // Dump stacks
  if (pfnDumpAllStacks) {
    pfnDumpAllStacks(env, NULL);    
  }
 
  /* file descriptor 1 point back to sfildes */ 
  _dup2(sfildes, 1); 
  
  /* Move the file pointer to the begining */ 
  _lseek(tfildes, 0, SEEK_SET); 
  
  /* Get the file size */ 
  fstat(tfildes, &buf); 
  len = buf.st_size; 
  
  /* Read the content in tmp file into output buffer */ 
  if (len > 0) { 
    pszConsoleOutput = (char *) calloc(len + 1, sizeof(char)); 
    read(tfildes, pszConsoleOutput, len); 
  } 
  
  /* Display the thread output in Java Console */ 
  if (pszConsoleOutput != NULL) {
    str = env->NewStringUTF(pszConsoleOutput); 
  }

  /* Release memory */ 
  free(pszConsoleOutput);

  close(tfildes);
  
  return str;
}

} /* extern "C" */
