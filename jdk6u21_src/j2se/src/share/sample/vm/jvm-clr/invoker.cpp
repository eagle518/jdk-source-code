/*
* @(#)invoker.cpp	1.2 10/03/23
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

#define _WIN32_WINNT 0x0400
#include "windows.h"
#include "Oleauto.h"
#include "stdio.h"
#include "mscoree.h"
#include "corerror.h"
#include "jni.h"
#include "invokerExp.h"
#include "invoker.h"

#import  <mscorlib.tlb> raw_interfaces_only 

using namespace mscorlib;

// The CLR assembly invocation function

int __stdcall invokeCLR( WCHAR* wszApplication){

    //Initializes the COM library
    
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    ICorRuntimeHost* pHost           = NULL;
    IUnknown*        pAppDomainThunk = NULL;
    _AppDomain*      pAppDomain      = NULL;
    long             lReturn         = 0;

    // Load CLR into the process
    
    HRESULT hr = CorBindToRuntimeEx(NULL,NULL,0,CLSID_CorRuntimeHost,IID_ICorRuntimeHost,(VOID**)&pHost);

    if(!FAILED(hr)) {

        // Start the CLR
        
        hr = pHost->Start();
        if(!FAILED(hr)) {

            // Get the _AppDomain interface

            hr = pHost->GetDefaultDomain(&pAppDomainThunk);
            if(!FAILED(hr)) {

                hr = pAppDomainThunk->QueryInterface(__uuidof(_AppDomain), (void**)&pAppDomain);
                if(!FAILED(hr)) {

                    // Execute assembly
                    
                    hr = pAppDomain->ExecuteAssembly_2(_bstr_t(wszApplication), &lReturn);
                    if (FAILED(hr)) {
                        
                        printf("_AppDomain::ExecuteAssembly_2 failed with hr=0x%x.\n", hr);
                        lReturn = -1;
                    }

                }else{
                    printf("Can't get System::_AppDomain interface\n");
                    lReturn = -2;
                }

            }else{
                printf("ICorRuntimeHost->GetDefaultDomain failed with hr=0x%x.\n", hr);
                lReturn = -3;
            }
        }else{
            printf("ICorRuntimeHost->Start failed with hr=0x%x.\n", hr);
            lReturn = -4;
        }

    }else{
        printf("CorBindToRuntimeHost failed with hr=0x%x.\n", hr);
        lReturn = -5;
    }
    
    // print the error message description if needed
    
    if(FAILED(hr)){
        LPVOID lpMsgBuf = NULL;

        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0,
                NULL );
        if(lpMsgBuf != NULL)
            printf("Message:%s\n",lpMsgBuf);
        else
            printf("No translation of 0x%x\n",hr);
    }

    // close COM library
    
    CoUninitialize();

    return lReturn;
}

// Wrapper function that allows to ASCIZ string to provide the assemble path

int __stdcall invokeCLR( const char* szApplication){

    int    nLength = strlen(szApplication)+1;
    
    WCHAR* wszApplication = new WCHAR[nLength];

    mbstowcs(wszApplication, szApplication, nLength);
  
    int nReturn = invokeCLR( wszApplication);

    delete wszApplication;

    return nReturn;
}

// native method enter-point

JNIEXPORT jint JNICALL Java_invoker_invokeCLR( JNIEnv* pEnv,
                                               jclass  pClass,
                                               jstring jsApplication) {
	
    const char* szApplication = pEnv->GetStringUTFChars(jsApplication, NULL);

    int nResult = invokeCLR( szApplication);

    pEnv->ReleaseStringUTFChars(jsApplication,szApplication);

    return nResult;
}
