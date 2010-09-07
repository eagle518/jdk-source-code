/*
 * @(#)auth.cpp	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// auth.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <shlobj.h>
#include <wininet.h>
#include <jni.h>
#include <ntsecapi.h>

#import "pstorec.tlb" raw_interfaces_only  no_namespace

#ifndef PSTORE_DLL
#define PSTORE_DLL	"pstorec.dll"
#endif

typedef HRESULT (WINAPI *PStoreCreateInstancePtr)(IPStore**, DWORD, DWORD, DWORD);

typedef NTSTATUS 
(NTAPI *LPFNEnumerateLogonSessions)(OUT PULONG, OUT PLUID* );

typedef NTSTATUS 
(NTAPI *LPFNGetLogonSessionData)(IN PLUID, 
                                 OUT PSECURITY_LOGON_SESSION_DATA* );

typedef NTSTATUS (NTAPI *LPFNFreeReturnBuffer)(IN PVOID Buffer);   

typedef BOOL
(WINAPI* LPFNCryptProtectData)(IN DATA_BLOB*, IN LPCWSTR, IN OPTIONAL DATA_BLOB*,
                               IN PVOID, IN OPTIONAL CRYPTPROTECT_PROMPTSTRUCT*,
                               IN DWORD, OUT DATA_BLOB* );

typedef BOOL 
(WINAPI* LPFNCryptUnprotectData)(IN DATA_BLOB*, OUT OPTIONAL LPWSTR*,
                                 IN OPTIONAL DATA_BLOB*, IN PVOID, 
                                 IN OPTIONAL CRYPTPROTECT_PROMPTSTRUCT*,
                                 IN DWORD dwFlags, OUT DATA_BLOB* );

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
(JNIEnv *env, jobject, jstring host, jint port, jboolean isSecure, 
	jstring path, jboolean proxyRequest) {

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

		// Now send HTTP Request only if OpenRequest returned a handle and 
		// a secure protocol is not being used 
		HINTERNET hReq = HttpOpenRequest(hConn, HTTP_METHOD_HEAD, lpszPath, NULL, NULL, lppszAccepts, 0, NULL);
		if(NULL != hReq && !isSecure && HttpSendRequest(hReq, NULL, 0, NULL,0)) {
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

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_security_MSCredentialManager_isEncryptionAvailable
(JNIEnv *env, jobject ) 
{
    jboolean result = FALSE;
    HMODULE hModule = NULL;  
    __try {           
        // Load the dll, return null if unavailable
        hModule = LoadLibrary( "Crypt32.dll" );
        if( hModule != NULL )
        {
            // Find the Function 
            if( GetProcAddress( hModule, "CryptProtectData" ) != NULL )
            {
                result = TRUE;
            } 
        }
    } __finally {
        if (hModule != NULL)
        {
            FreeLibrary(hModule);
        }         
    }
    return result;
}

JNIEXPORT jbyteArray JNICALL Java_com_sun_deploy_security_MSCredentialManager_encryptMSPassword
(JNIEnv *env, jobject, jcharArray szPass ) 
{ 
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    HMODULE hModule = NULL;
    jbyteArray result = NULL;
    LPFNCryptProtectData pfnCryptProtectData;

    __try {
        hModule = LoadLibrary( "Crypt32.dll" );
        if( hModule != NULL ) 
        {
            pfnCryptProtectData = (LPFNCryptProtectData)
                                 GetProcAddress( hModule, "CryptProtectData" );
        }

        if( pfnCryptProtectData )
        {
            DataIn.pbData = (BYTE*)env->GetCharArrayElements(szPass, NULL);    
            DataIn.cbData = env->GetArrayLength(szPass)*2;

            //  Begin protect phase. The text description field must be present on
            //  some platforms though it doesn't do anything as we don't use the
            //  Windows GUI.
            if( pfnCryptProtectData( &DataIn, L"Password Data", 
                    NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &DataOut))
            {
                // Copy the data to the Java Object and free the structure
                result = env->NewByteArray( DataOut.cbData );
                if(result != NULL) 
                {
                    const signed char* data = (const signed char*)DataOut.pbData;
                    env->SetByteArrayRegion(result, 0, DataOut.cbData, data );
                    LocalFree(DataOut.pbData);
                }
            }
            env->ReleaseCharArrayElements( szPass, (jchar*)DataIn.pbData, 0 );
         }
    } __finally {
        if (hModule != NULL)
        {
            FreeLibrary(hModule);
        }         
    } 
    return result;
}

JNIEXPORT jcharArray JNICALL Java_com_sun_deploy_security_MSCredentialManager_decryptMSPassword
(JNIEnv *env, jobject, jbyteArray szEncrypted ) 
{
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    jcharArray result = NULL;
    HMODULE hModule = NULL;
    LPFNCryptUnprotectData pfnCryptUnprotectData;
   
    __try {
        hModule = LoadLibrary( "Crypt32.dll" );
        if( hModule != NULL ) 
        {
            pfnCryptUnprotectData = (LPFNCryptUnprotectData)
                                GetProcAddress( hModule, "CryptUnprotectData" );
        }

        if( pfnCryptUnprotectData )
        {
            DataIn.pbData = (BYTE*)env->GetByteArrayElements(szEncrypted, NULL);    
            DataIn.cbData = env->GetArrayLength(szEncrypted);

            if ( pfnCryptUnprotectData( &DataIn, NULL, NULL, NULL, NULL, 0, &DataOut) )
            {
                result = env->NewCharArray( DataOut.cbData/2 );
                if (result != NULL) 
                {
                    const jchar* data = (jchar*)DataOut.pbData;

                    // Copy decrypted data into Java Array
                    env->SetCharArrayRegion( result, 0, DataOut.cbData/2, data );

                    LocalFree( DataOut.pbData );
                 }
            }
            env->ReleaseByteArrayElements( szEncrypted, (jbyte*)DataIn.pbData, 0 );    
        }
    }
    __finally {
        if (hModule != NULL)
        {
            FreeLibrary(hModule);
        }         
    } 
	
    return result;
}

JNIEXPORT jlong JNICALL Java_com_sun_deploy_security_MSCredentialManager_getLoginUID
(JNIEnv *env, jobject) {
    jlong result = 0L;    
    HMODULE hModule = NULL;  
    LPFNEnumerateLogonSessions pfnEnumLogonSessions;
    LPFNGetLogonSessionData pfnGetLogonSessionData;
    LPFNFreeReturnBuffer pfnFreeReturnBuffer;
 
    __try {
	PLUID sessionIdentifiers;
	unsigned long totalSessions;
        const int AUTH_NAME_MAX = 255;
	DWORD USER_NAME_SIZE = AUTH_NAME_MAX;
	wchar_t userName[AUTH_NAME_MAX];

        // Initialize to any non ERROR_SUCCESS value
        long status = -1L;
        
        hModule = LoadLibrary( "Secur32.dll" );

        // Get function pointers
        pfnEnumLogonSessions = (LPFNEnumerateLogonSessions)
	              GetProcAddress( hModule, "LsaEnumerateLogonSessions");
        pfnGetLogonSessionData = (LPFNGetLogonSessionData)
                      GetProcAddress( hModule, "LsaGetLogonSessionData");
        pfnFreeReturnBuffer = (LPFNFreeReturnBuffer)
                      GetProcAddress( hModule, "LsaFreeReturnBuffer");

        // Get a list of active logon sessions if possible
        if( pfnEnumLogonSessions != NULL ) 
        {
            status = pfnEnumLogonSessions(&totalSessions, &sessionIdentifiers);
        }

	// Just use the first session if enumeration fails or is unavailable
	if(status != ERROR_SUCCESS)
	{
	    totalSessions = 1;
	} 
        
	// Retrieve the logon session information for each session identifier.
	for(int i = 0; i < (int) totalSessions; i++)
	{
	    // Get the session data for the current session.
	    PSECURITY_LOGON_SESSION_DATA sessionData = NULL;

            if( pfnGetLogonSessionData != NULL )
	    {
                status =
		 pfnGetLogonSessionData( &sessionIdentifiers[i], &sessionData );
            } else {
                // Bad stuff
                status = -1L;
            }

	    if(status != ERROR_SUCCESS)
	    {
	        // Failure.
		break;
	    } 

	    // If an authentication package name exists...
	    if(sessionData->AuthenticationPackage.Buffer != NULL)
	    {                   
	        GetUserNameW(userName, &USER_NAME_SIZE);
		
                // Loop through until we find the one associated with 
                // the active User Name
		if( !wcscmp(sessionData->UserName.Buffer, userName ))
		{ 
                    LUID lid = sessionData->LogonId;
                    result = lid.LowPart;
                    // Break out of loop
		    i = totalSessions;
		}
            }
            if( sessionData != NULL) 
            {
		    // Free the session data buffer.
		    pfnFreeReturnBuffer(sessionData);
	    } 
	}
	// Free the session ID buffer.
        if( pfnFreeReturnBuffer ) 
        {
	   pfnFreeReturnBuffer(sessionIdentifiers);
        }

	if (hModule != NULL) {
            FreeLibrary(hModule);
    	}         
    } 
    __except (EXCEPTION_EXECUTE_HANDLER) {
        if (hModule != NULL) {
            FreeLibrary(hModule);
        }         
        return NULL;
    } 
    return result;
  }

}
