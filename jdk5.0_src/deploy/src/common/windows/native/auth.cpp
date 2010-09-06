/*
 * @(#)auth.cpp	1.5 04/02/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// auth.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <shlobj.h>
#include <wininet.h>
#include <atlcom.h>
#include <jni.h>

#import "pstorec.tlb" raw_interfaces_only  no_namespace

#ifndef PSTORE_DLL
#define PSTORE_DLL	"pstorec.dll"
#endif

typedef HRESULT (WINAPI *PStoreCreateInstancePtr)(IPStore**, DWORD, DWORD, DWORD);

// Winnet Cache credential
static GUID GUID_WINNET_CACHE_CREDENTIAL =
{0x5E7E8100, 0x9138, 0x11D1, {0x94, 0x5A, 0x00, 0xC0, 0x4F, 0xc3, 0x08, 0xFF}};
static GUID GUID_WINNET_CACHE_CREDENTIAL_SUB = 
{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};


PStoreCreateInstancePtr GetPSCreateProc() {

	// IE normally will load the DLL by now
	HMODULE hPStoreDLL = ::GetModuleHandle(PSTORE_DLL);
	if(NULL == hPStoreDLL)
		hPStoreDLL = ::LoadLibrary(PSTORE_DLL);

	if(NULL == hPStoreDLL)
		return NULL;

	return (PStoreCreateInstancePtr)::GetProcAddress(hPStoreDLL, "PStoreCreateInstance");
}


#define HTTP_ACCEPT_MIME		"*/*"
#define HTTP_METHOD_HEAD		"HEAD"

extern "C"
{

static HINTERNET s_hInternet = NULL;

JNIEXPORT jobject JNICALL Java_com_sun_deploy_security_WIExplorerBrowserAuthenticator_getAuthFromInet
(JNIEnv *env, jobject, jstring host, jint port, jstring path, jboolean proxyRequest) {

	if(NULL == s_hInternet) {
		s_hInternet = ::InternetOpen(NULL,
						INTERNET_OPEN_TYPE_PRECONFIG,
						NULL,
						NULL,
						0);
		if(NULL == s_hInternet)	// something wrong
			return NULL;
	}

	LPCSTR lpszHost = env->GetStringUTFChars(host, (jboolean*)0);


	HINTERNET  hConn = InternetConnect(s_hInternet, lpszHost, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	env->ReleaseStringUTFChars(host, lpszHost);

	jobject jRet = NULL;

	if(hConn != NULL) {
		LPCSTR lpszPath = env->GetStringUTFChars(path, (jboolean*)0);
		LPCSTR lppszAccepts[2];
		lppszAccepts[0] = HTTP_ACCEPT_MIME;
		lppszAccepts[1] = NULL;


		HINTERNET hReq = HttpOpenRequest(hConn, HTTP_METHOD_HEAD, lpszPath, NULL, NULL, lppszAccepts, 0, NULL);
		if(NULL != hReq && ::HttpSendRequest(hReq, NULL, 0, NULL, 0)) {
			char szServerUserName[MAX_PATH];
			char szServerPassword[MAX_PATH];
			char szProxyUserName[MAX_PATH];
			char szProxyPassword[MAX_PATH];

			ZeroMemory(szServerUserName, sizeof(szServerUserName));
			ZeroMemory(szServerPassword, sizeof(szServerPassword));
			ZeroMemory(szProxyUserName, sizeof(szProxyUserName));
			ZeroMemory(szProxyPassword, sizeof(szProxyPassword));

			DWORD cbServerUserName = sizeof(szServerUserName);
			DWORD cbServerPassword = sizeof(szServerPassword);
			DWORD cbProxyUserName = sizeof(szProxyUserName);
			DWORD cbProxyPassword = sizeof(szProxyPassword);

			jcharArray jServerAuth = NULL;
			jcharArray jProxyAuth = NULL;
			
			USES_CONVERSION;
			if(InternetQueryOption(hReq, INTERNET_OPTION_USERNAME, (LPVOID)szServerUserName, &cbServerUserName) &&
				InternetQueryOption(hReq, INTERNET_OPTION_PASSWORD, (LPVOID)szServerPassword, &cbServerPassword)) {
				if(cbServerUserName > 0) {
					jServerAuth = env->NewCharArray(wcslen(A2W(szServerUserName)) + wcslen(A2W(szServerPassword)) + 1);
					LPWSTR lpwTmp = A2W(szServerUserName);
					int nLen = wcslen(lpwTmp);
					env->SetCharArrayRegion(jServerAuth, 0, nLen, (jchar*)lpwTmp);
					env->SetCharArrayRegion(jServerAuth, nLen, 1, L":");
					lpwTmp = A2W(szServerPassword);
					env->SetCharArrayRegion(jServerAuth, nLen + 1, wcslen(lpwTmp), (jchar*)lpwTmp);
				}
			}


			if(InternetQueryOption(hReq, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)szProxyUserName, &cbProxyUserName) &&
				InternetQueryOption(hReq, INTERNET_OPTION_PROXY_PASSWORD , (LPVOID)szProxyPassword, &cbProxyPassword)) {
				if(cbProxyUserName > 0) {
					jProxyAuth = env->NewCharArray(wcslen(A2W(szProxyUserName)) + wcslen(A2W(szProxyPassword)) + 1);
					LPWSTR lpwTmp = A2W(szProxyUserName);
					int nLen = wcslen(lpwTmp);
					env->SetCharArrayRegion(jProxyAuth, 0, nLen, (jchar*)lpwTmp);
					env->SetCharArrayRegion(jProxyAuth, nLen, 1, L":");
					lpwTmp = A2W(szProxyPassword);
					env->SetCharArrayRegion(jProxyAuth, nLen + 1, wcslen(lpwTmp), (jchar*)lpwTmp);
				}
			} 

			if(NULL != jServerAuth || NULL != jProxyAuth) {
				jclass AuthInfoItemClass = env->FindClass("com/sun/deploy/security/AuthInfoItem");
				if(NULL != AuthInfoItemClass) {
					jmethodID mthdInit = env->GetMethodID(AuthInfoItemClass, "<init>", "([C[C)V");
					if(NULL != mthdInit)
						jRet = env->NewObject(AuthInfoItemClass, mthdInit, jServerAuth, jProxyAuth);

				}
			}
			
			InternetCloseHandle(hReq);
		}

		env->ReleaseStringUTFChars(path, lpszPath);
		InternetCloseHandle(hConn);
	} 

	return jRet;
}

	
JNIEXPORT jcharArray JNICALL Java_com_sun_deploy_security_WIExplorerBrowserAuthenticator14_getAuthentication
(JNIEnv *env, jobject, jstring key) {

	__try {
		IPStore* pIPStore = NULL;
		IEnumPStoreItems* pIEnumPStoreItems = NULL;

		PStoreCreateInstancePtr PStoreCreateInstance = GetPSCreateProc();

		if(NULL == PStoreCreateInstance)
			return NULL;

		HRESULT hr = PStoreCreateInstance(&pIPStore, 0, 0, 0);
		if(FAILED(hr))
			return NULL;

		if(FAILED(pIPStore->EnumItems(0, &GUID_WINNET_CACHE_CREDENTIAL, &GUID_WINNET_CACHE_CREDENTIAL_SUB, 0, &pIEnumPStoreItems))) {
			pIPStore->Release();
			return NULL;
		}

		// It only uses server and realm as key
		const char* lpszKey = env->GetStringUTFChars(key, (jboolean*)0);

		// Has to convert to wide char
		USES_CONVERSION;
		LPCWSTR lpcwKey = A2CW(lpszKey);
		env->ReleaseStringUTFChars(key, lpszKey);

		LPWSTR szKey = NULL;
		jcharArray ret = NULL;

		while(SUCCEEDED(pIEnumPStoreItems->Next(1, &szKey, NULL))) {
			// compare server/realm
			if(0 != wcscmp(lpcwKey, szKey)) {
				::CoTaskMemFree(szKey);
				szKey = NULL;
				continue;
			}
			::CoTaskMemFree(szKey);
			szKey = NULL;

			unsigned long cbData = 0;
			unsigned char* pbData = NULL;
			if(SUCCEEDED(pIPStore->ReadItem(0, &GUID_WINNET_CACHE_CREDENTIAL, &GUID_WINNET_CACHE_CREDENTIAL_SUB, (LPWSTR)lpcwKey, &cbData, 
				(char**)&pbData, NULL, NULL))) {
					char* pAuth = (char*)pbData;
					if(NULL == pAuth)
						break;

					LPCWSTR lpcwAuth = A2CW(pAuth);

					// create java char array
					int slen = strlen(pAuth);
					ret = env->NewCharArray(slen);
					env->SetCharArrayRegion(ret, 0, slen, (jchar*)lpcwAuth);
					::CoTaskMemFree(pbData);
					break;
			}
		}
		pIPStore->Release();
		pIEnumPStoreItems->Release();

		return ret;
	}
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return NULL;
}

}
