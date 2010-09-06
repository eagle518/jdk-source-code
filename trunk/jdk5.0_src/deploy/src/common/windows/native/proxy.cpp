/*
 * @(#)proxy.cpp	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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


extern "C"
{

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

