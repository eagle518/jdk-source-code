/*
* @(#)jinvoker.cpp	1.2 10/03/23
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

#include <memory.h>
#include <stdlib.h>
#include "jinvokerExp.h"

static int g_nExitCode = 0;

void system_exit(jint nCode){
    g_nExitCode = nCode;
}

/*
Allocating and providing the JVM init argumets.
By MakeJavaVMInitArgs() it is provided two options: providing CLASSPATH
environment variable value and function java.lang.System.exit()
redefinition in order to get the exit code.
See the description of the JNI API in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/invocation.html#wp9502
*/

int MakeJavaVMInitArgs( void** ppArgs ){

    int nOptSize = 2;
    JavaVMInitArgs* pArgs    = new JavaVMInitArgs();
    JavaVMOption*   pOptions = new JavaVMOption[nOptSize];

    //provide CLASSPATH value to java.class.path 

    char* szClassPath = getenv("CLASSPATH");
    if( szClassPath == NULL )
        szClassPath = ".";
	
    pOptions[0].optionString = new char[strlen("-Djava.class.path=")+
                                        strlen(szClassPath)+1];
    sprintf( pOptions[0].optionString, "-Djava.class.path=%s", szClassPath );

    //redefine java.lang.System.exit()
    
    pOptions[1].optionString = "exit";
    pOptions[1].extraInfo    = system_exit;

    //Fill the arguments
    
    memset(pArgs, 0, sizeof(JavaVMInitArgs));
    pArgs->version = 0x00010002;
    pArgs->options = pOptions;
    pArgs->nOptions = nOptSize;
    pArgs->ignoreUnrecognized = JNI_TRUE;
	
    *ppArgs = pArgs;

    return 0;
}

/*
Free the allocated JVM init argumets
*/

void FreeJavaVMInitArgs( void* pArgs ){
    delete ((JavaVMInitArgs*)pArgs)->options[0].optionString;
    delete ((JavaVMInitArgs*)pArgs)->options;
    delete pArgs;
}

/*
Static wrapper on FindClass() JNI function.
See the description in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/functions.html#wp16027
*/

int FindClass( JNIEnv*     pEnv,
               const char* szClass,
               jclass*     pClass ){

    *pClass = pEnv->FindClass( szClass );

    if(pEnv->ExceptionCheck() == JNI_TRUE){
        pEnv->ExceptionDescribe();
        return -1;
    }
    if(*pClass != NULL)
        return 0;
    else
        return -2;

}

/*
Static wrapper on GetStaticMethodID() JNI function.
See the description in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/functions.html#wp20949
*/

int GetStaticMethodID(JNIEnv*     pEnv,
                      jclass      pClass, 
                      const char* szName, 
                      const char* szArgs, 
                      jmethodID*  pMid){
	
    *pMid = pEnv->GetStaticMethodID( pClass, szName, szArgs);
	
    if(pEnv->ExceptionCheck() == JNI_TRUE){
        pEnv->ExceptionDescribe();
        return -1;
    }
    
    if( *pMid != NULL )
        return 0;
    else
        return -2;
}

/*
Static wrapper on NewObjectArray() JNI function.
See the description in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/functions.html#wp21619
*/

int NewObjectArray( JNIEnv*       pEnv,
                    int           nDimension,
                    const char*   szType,
                    jobjectArray* pArray ){
	
    *pArray = pEnv->NewObjectArray( nDimension, pEnv->FindClass( szType ), NULL);

    if(pEnv->ExceptionCheck() == JNI_TRUE){
        pEnv->ExceptionDescribe();
        return -1;
    }
    
    if( pArray != NULL )
        return 0;
    else
        return -2;

}

/*
Static wrapper on CallStaticVoidMethod() JNI function.
See the description in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/functions.html#wp4796
*/

int CallStaticVoidMethod( JNIEnv*   pEnv,
                          jclass    pClass,
                          jmethodID pMid,
                          void*     pArgs){

    g_nExitCode = 0;
    pEnv->CallStaticVoidMethod( pClass, pMid, pArgs);
    if( pEnv->ExceptionCheck() == JNI_TRUE ){
        pEnv->ExceptionDescribe();
        return -1;
    }else
        return g_nExitCode;
}

/*
Static wrapper on DestroyJavaVM() JNI function.
See the description in
http://jre.sfbay/java/re/jdk/6/promoted/latest/docs/technotes/guides/jni/spec/invocation.html#destroy_java_vm
*/

int DestroyJavaVM( JavaVM* pJVM ){
    pJVM->DestroyJavaVM();
    return 0;
}
