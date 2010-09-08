/*
* @(#)jinvokerExp.h	1.2 10/03/23
*
* Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* -Redistribution of source code must retain the above copyright notice, this
*  list of conditions and the following disclaimer.
*
* -Redistribution in binary form must reproduce the above copyright notice,
*  this list of conditions and the following disclaimer in the documentation
*  and/or other materials provided with the distribution.
*
* Neither the name of Oracle or the names of contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* This software is provided "AS IS," without a warranty of any kind. ALL
* EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
* ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MICROSYSTEMS, INC. ("SUN")
* AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
* AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
* DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST
* REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
* INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY
* OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
* EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* You acknowledge that this software is not designed, licensed or intended
* for use in the design, construction, operation or maintenance of any
* nuclear facility.
*/

#include <windows.h>
#include <jni.h>

#ifdef JINVOKEREEXPORT
#define JINVOKERAPI __declspec(dllexport)
#else
#define JINVOKERAPI __declspec(dllimport)
#endif

// Create JNI_CreateJavaVM() args structures
extern "C" int  JINVOKERAPI MakeJavaVMInitArgs( void** ppArgs );

// Free JNI_CreateJavaVM() args structures
extern "C" void JINVOKERAPI FreeJavaVMInitArgs( void* pArgs );

// Static wrapper on FindClass() JNI function.
extern "C" int  JINVOKERAPI FindClass( JNIEnv* pEnv, 
                                       const char* szName, 
                                       jclass*     ppClass );

// Static wrapper on GetStaticMethodID() JNI function.
extern "C" int JINVOKERAPI GetStaticMethodID( JNIEnv*     pEnv,
                                              jclass      pClass, 
                                              const char* szName, 
                                              const char* szArgs, 
                                              jmethodID*  ppMid );

// Static wrapper on NewObjectArray() JNI function.
extern "C" int JINVOKERAPI NewObjectArray( JNIEnv*       pEnv,
                                           int           nDimension,
                                           const char*   szType,
                                           jobjectArray* pArray );

// Static wrapper on CallStaticVoidMethod() JNI function.
extern "C" int JINVOKERAPI CallStaticVoidMethod( JNIEnv*   pEnv,
                                                 jclass    pClass,
                                                 jmethodID pMid,
                                                 void*     pArgs);

// Static wrapper on DestroyJavaVM() JNI function.
extern "C" int JINVOKERAPI DestroyJavaVM( JavaVM* pEnv );

