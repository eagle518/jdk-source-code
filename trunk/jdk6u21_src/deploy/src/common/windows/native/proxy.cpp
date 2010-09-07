/*
 * @(#)proxy.cpp	1.10 10/03/31
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// proxy.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <activscp.h>
#include <comdef.h>	   // COM smart pointer
#include <jni.h>
#include <Wininet.h>

#include "jscriptengine.h"

// These are defined in a newer version of WinInet.h
// When The JDK build environment is updated to uses the newer version
// of the MS Platform SDK this will cause a compile error and should be
// removed. 
#define  PROXY_AUTO_DETECT_TYPE_DHCP    1
#define  PROXY_AUTO_DETECT_TYPE_DNS_A   2

typedef BOOL (CALLBACK * pfnDetectAutoProxyUrl)(
    IN OUT LPSTR lpszAutoProxyUrl,
    IN DWORD dwAutoProxyUrlLength,
    IN DWORD dwDetectFlags
    );


extern "C"
{

/*
 * Class:     com_sun_deploy_net_proxy_WMozillaAutoProxyHandler
 * Method:    evalJScript
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_proxy_WMozillaAutoProxyHandler_evalScript
  (JNIEnv *env, jobject, jstring jScript)
{
    const wchar_t* lpwszScript = NULL;
    wchar_t wszResult[1024] = L"";
    jstring result = NULL;

    __try
    {
	::CoInitialize(NULL);	// Initialize COM

	// Load Javascript engine
	JScriptEngine jscriptEngine;

	lpwszScript = env->GetStringChars(jScript, JNI_FALSE);

	if (SUCCEEDED(jscriptEngine.Eval(lpwszScript, wszResult)))
	    result = env->NewString(wszResult, lstrlenW(wszResult));
    }
    __finally
    {
	if (lpwszScript != NULL)
	    env->ReleaseStringChars(jScript, lpwszScript);

	::CoUninitialize();	// Uninitialize COM
    }

    return result;
}

/*
 * Class:     com_sun_deploy_net_proxy_WIExplorerAutoProxyHandler
 * Method:    evalJScript
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_proxy_WIExplorerAutoProxyHandler_evalScript
  (JNIEnv *env, jobject, jstring jScript)
{
    const wchar_t* lpwszScript = NULL;
    wchar_t wszResult[1024] = L"";
    jstring result = NULL;

    __try
    {
	::CoInitialize(NULL);	// Initialize COM

	// Load Javascript engine
	JScriptEngine jscriptEngine;

	lpwszScript = env->GetStringChars(jScript, JNI_FALSE);

	if (SUCCEEDED(jscriptEngine.Eval(lpwszScript, wszResult)))
	    result = env->NewString(wszResult, lstrlenW(wszResult));
    }
    __finally
    {
	if (lpwszScript != NULL)
	    env->ReleaseStringChars(jScript, lpwszScript);

	::CoUninitialize();	// Uninitialize COM
    }

    return result;
}

/*
 * Class:     com_sun_deploy_net_proxy_WIExplorerProxyConfig
 * Method:    performAutoDetection
 * Signature: ()Lcom/sun/deploy/net/proxy/WIExplorerProxyConfig;
 */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_proxy_WIExplorerProxyConfig_performAutoDetection
(JNIEnv *env, jobject) {

    // Initialize Variables
    HMODULE hModWinInet				= NULL;
    jstring result				= NULL;
    pfnDetectAutoProxyUrl pDetectAutoProxyUrl	= NULL;
    char WPADLocation[1024];
      
    // Load the dll, return null if unavailable
    if( !(hModWinInet = LoadLibrary( "wininet.dll" )) )
    {
        return result;
    }

    // Find the Function in the wininet.dll
    pDetectAutoProxyUrl = (pfnDetectAutoProxyUrl)
	              GetProcAddress( hModWinInet, "DetectAutoProxyUrl" );;

    // Verify the DLL has the function we are calling
    if( pDetectAutoProxyUrl == NULL )
    {
        FreeLibrary( hModWinInet );

        // Older version of Dll 
        // so we return NULL
        return result;
    }
  
    BOOL bResult = false;
  
    // Find the WPAD location using DetectAutoProxyUrl
    bResult = pDetectAutoProxyUrl( WPADLocation,
                      sizeof(WPADLocation), 
                      PROXY_AUTO_DETECT_TYPE_DHCP | PROXY_AUTO_DETECT_TYPE_DNS_A );

    // If an AutoProxy URL was found copy it into the Java Object
    if( bResult )
    {
        result = env->NewStringUTF(WPADLocation);
    }

    // Unload the Library and return the url
    FreeLibrary( hModWinInet );
		
    return result;

}


/*
 * Class:     com_sun_deploy_net_proxy_WIExplorerProxyConfig
 * Method:    getBrowserProxySettings
 * Signature: ()Lcom/sun/deploy/net/proxy/WIExplorerProxyConfig$ProxyInfo;
 */
JNIEXPORT jobject JNICALL Java_com_sun_deploy_net_proxy_WIExplorerProxyConfig_getBrowserProxySettings
(JNIEnv *env, jobject) {
	INTERNET_PROXY_INFO* lpProxyInfo;
	DWORD dwLength = 4096;
	char* lpszBuf = new char[dwLength];
	jobject result = NULL;
	ZeroMemory(lpszBuf, dwLength);

	BOOL bRet = InternetQueryOption(NULL, INTERNET_OPTION_PROXY , (LPVOID)lpszBuf, &dwLength);
	if(bRet) {
		lpProxyInfo = (INTERNET_PROXY_INFO*)lpszBuf;
	    jclass proxyInfoClass = env->FindClass("com/sun/deploy/net/proxy/WIExplorerProxyConfig$ProxyInfo");

	    if (proxyInfoClass == NULL) {
			delete[] lpszBuf;
			return NULL;  /* exception thrown */
		}

		jmethodID mthdInit = env->GetMethodID(proxyInfoClass, "<init>", "(ILjava/lang/String;Ljava/lang/String;)V");
		if (mthdInit == NULL) {
			delete[] lpszBuf;
			return NULL;  /* exception thrown */
		}

		jstring jstrProxy = NULL;
		if(NULL != lpProxyInfo->lpszProxy)
			jstrProxy = env->NewStringUTF(lpProxyInfo->lpszProxy);
		jstring jstrProxyBypass = NULL;
		if(NULL != lpProxyInfo->lpszProxyBypass)
			jstrProxyBypass = env->NewStringUTF(lpProxyInfo->lpszProxyBypass);
		result = env->NewObject(proxyInfoClass, mthdInit, (jint)lpProxyInfo->dwAccessType, jstrProxy, jstrProxyBypass);
	}
	delete[] lpszBuf;
	return result;
}

}

