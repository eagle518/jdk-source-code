/*
 * @(#)WinCAPISeedGenerator.c	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Need to define this to get CAPI functions included */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>
#include <wincrypt.h>
#include <jni.h>
#include "sun_security_provider_NativeSeedGenerator.h"

/* Typedefs for runtime linking. */
typedef BOOL (WINAPI *CryptAcquireContextType)(HCRYPTPROV*, LPCTSTR, LPCTSTR, DWORD, DWORD);
typedef BOOL (WINAPI *CryptGenRandomType)(HCRYPTPROV, DWORD, BYTE*);
typedef BOOL (WINAPI *CryptReleaseContextType)(HCRYPTPROV, DWORD);

/*
 * Get a random seed from the MS CryptoAPI. Return true if successful, false
 * otherwise.
 *
 * Some early versions of Windows 95 do not support the required functions.
 * Use runtime linking to avoid problems.
 *
 * @version 1.3, 12/19/03
 */
JNIEXPORT jboolean JNICALL Java_sun_security_provider_NativeSeedGenerator_nativeGenerateSeed
  (JNIEnv *env, jclass clazz, jbyteArray randArray)
{
    HMODULE lib;
    CryptAcquireContextType acquireContext;
    CryptGenRandomType genRandom;
    CryptReleaseContextType releaseContext;

    HCRYPTPROV hCryptProv;
    jboolean result = JNI_FALSE;
    jsize numBytes;
    jbyte* randBytes;

    lib = LoadLibrary("ADVAPI32.DLL");
    if (lib == NULL) {
	return result;
    }

    acquireContext = (CryptAcquireContextType)GetProcAddress(lib, "CryptAcquireContextA");
    genRandom = (CryptGenRandomType)GetProcAddress(lib, "CryptGenRandom");
    releaseContext = (CryptReleaseContextType)GetProcAddress(lib, "CryptReleaseContext");

    if (acquireContext == NULL || genRandom == NULL || releaseContext == NULL) {
	FreeLibrary(lib);
	return result;
    }

    if (acquireContext(&hCryptProv, "J2SE", NULL, PROV_RSA_FULL, 0) == FALSE) {
	/* If CSP context hasn't been created, create one. */
	if (acquireContext(&hCryptProv, "J2SE", NULL, PROV_RSA_FULL,
		CRYPT_NEWKEYSET) == FALSE) {
	    FreeLibrary(lib);
	    return result;
	}
    }

    numBytes = (*env)->GetArrayLength(env, randArray);
    randBytes = (*env)->GetByteArrayElements(env, randArray, NULL);
    if (genRandom(hCryptProv, numBytes, randBytes)) {
	result = JNI_TRUE;
    }
    (*env)->ReleaseByteArrayElements(env, randArray, randBytes, 0);

    releaseContext(hCryptProv, 0);
    FreeLibrary(lib);

    return result;
}
