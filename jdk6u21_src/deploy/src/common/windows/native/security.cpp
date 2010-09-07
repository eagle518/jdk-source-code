/*
 * @(#)security.cpp	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// security.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <jni.h>
#include <windows.h>
#include <BaseTsd.h>
#include <Wincrypt.h>
#include <stdio.h>

#define OID_EKU_CLIENT_AUTH "1.3.6.1.5.5.7.3.2"

extern "C" {

/////////////////////////////////////////////////////////////////////////////
//

/*
 * Class:     com_sun_deploy_security_WSeedGenerator
 * Method:    generateSeed
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_sun_deploy_security_WSeedGenerator_generateSeed
  (JNIEnv *env, jclass, jint num)
{
    HCRYPTPROV hCryptProv = NULL;
    BYTE*        pbData = new BYTE[num];
    jbyteArray	 result = NULL;
  
    __try
    {
	//  Acquire a CSP context.
	if(::CryptAcquireContext(    
	   &hCryptProv,
	   "JavaWebStart",
	   NULL,
	   PROV_RSA_FULL,
	   0) == FALSE) 
	{
	    // If CSP context hasn't been created, create one.
	    //
	    if (::CryptAcquireContext(    
	        &hCryptProv,
	        "JavaWebStart",
	        NULL,
	        PROV_RSA_FULL,
	        CRYPT_NEWKEYSET) == FALSE)
	    {
		__leave;
	    }
	}

	// Generate a random initialization vector.
	if(::CryptGenRandom(
	   hCryptProv, 
	   num, 
	   pbData) == FALSE) 
	{
	    __leave;
	}

	result = env->NewByteArray(num);
	env->SetByteArrayRegion(result, 0, num, (jbyte*) pbData);
    }
    __finally
    {
	//--------------------------------------------------------------------
	// Clean up.

	if (pbData)
	    delete [] pbData;
    
	if (hCryptProv)
	    ::CryptReleaseContext(hCryptProv, 0);
    }

    return result;
}

/**
 * Return true if certificate in certificate context is valid for
 * a particular set of OID key usage.
 */
bool IsKeyUsageValid(PCCERT_CONTEXT pCertContext, LPCSTR pszEkuOidFilters)
{
    // Check if filter is for all usages
    if (strstr(pszEkuOidFilters, "2.5.29.37.0") != NULL)
	return true;

    // Usage filter is for a specific usage
    DWORD cbUsage = 0;
    PCERT_ENHKEY_USAGE pUsage = NULL;

    __try
    {
	// Determine length
	if (::CertGetEnhancedKeyUsage(pCertContext, 0, NULL, &cbUsage))
	{
	    pUsage = (PCERT_ENHKEY_USAGE) new byte[cbUsage];

	    // Extract Extended Key Usage Extension and Extended Property Value
	    if (::CertGetEnhancedKeyUsage(pCertContext, 0, pUsage, &cbUsage))
	    {
            
		// No usage is specific, so default to <Any>
		if (pUsage->cUsageIdentifier == 0)
		    return true;

		// Loop through usage arrays to see if it matches the filter
		for (int i=0; i < pUsage->cUsageIdentifier; i++)
		{
		    if (strstr(pszEkuOidFilters, pUsage->rgpszUsageIdentifier[i]) != NULL)
			return true;
		}
	    }
	}
	else
	{
	  // For Window NT, Window 98/95,
	  // CertGetEnhancedKeyUsage return false if the EKU extension is not found,
	  // A GetLastError return CRYPT_E_NOT_FOUND indicates that the certificate 
	  // is valid for all uses
	  if (GetLastError() == CRYPT_E_NOT_FOUND)
	     return true;
	}
    }
    __finally
    {
	if (pUsage != NULL)
	    delete [] pUsage;
    }
    
    return false;
}


/**
 * Return certificate chain context given a certificate context and key usage identifier.
 */
bool GetCertificateChain(LPSTR lpszKeyUsageIdentifier, PCCERT_CONTEXT pCertContext, PCCERT_CHAIN_CONTEXT* ppChainContext)
{
    CERT_ENHKEY_USAGE        EnhkeyUsage;
    CERT_USAGE_MATCH         CertUsage;  
    CERT_CHAIN_PARA          ChainPara;
    DWORD                    dwFlags = 0;
    LPSTR		     szUsageIdentifierArray[1];

    szUsageIdentifierArray[0] = lpszKeyUsageIdentifier;
    EnhkeyUsage.cUsageIdentifier = 1;
    EnhkeyUsage.rgpszUsageIdentifier = szUsageIdentifierArray;
    CertUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertUsage.Usage  = EnhkeyUsage;
    ChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    ChainPara.RequestedUsage=CertUsage;

    // Build a chain using CertGetCertificateChain
    // and the certificate retrieved.
    return (::CertGetCertificateChain(NULL,                // use the default chain engine
				pCertContext,          // pointer to the end certificate
				NULL,                  // use the default time
				NULL,                  // search no additional stores
				&ChainPara,            // use AND logic and enhanced key usage 
						       //  as indicated in the ChainPara 
						       //  data structure
				dwFlags,
				NULL,                  // currently reserved
				ppChainContext) == TRUE);       // return a pointer to the chain created
}
 
/*
 * Class:     com_sun_deploy_security_WIExplorerCertStore
 * Method:    loadCertificates
 * Signature: (Ljava/lang/String;[Ljava/lang/String;Ljava/util/Collection;)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_security_WIExplorerCertStore_loadCertificates
  (JNIEnv *env, jobject obj, jstring jCertStoreName, jobjectArray jEkuOidFilters, jobject jCertCollections)
{
    /**
     * Certificate stored in Internet Explorer cert store has enhanced key usage extension 
     * property (or EKU property) that is not part of the certificate itself. To determine
     * if the certificate should be returned, both the enhanced key usage in certificate 
     * extension block and the extension property stored along with the certificate in 
     * certificate store should be examined. Otherwise, we won't be able to determine
     * the proper key usage from the Java side because the information is not stored as
     * part of the encoded certificate.
     */

    const char* pszCertStoreName = NULL;
    char szEkuOidFilters[2048] = {0};
    HCERTSTORE hCertStore = NULL;        
    PCCERT_CONTEXT pCertContext=NULL; 

    __try
    {
	// Open a system certificate store.
        pszCertStoreName = env->GetStringUTFChars(jCertStoreName, (boolean*)0);
	if ((hCertStore = ::CertOpenSystemStore(NULL, pszCertStoreName)) == NULL)
	    __leave;

	// Determine enhanced key usage filters
	int filterSize = env->GetArrayLength(jEkuOidFilters);   
	for (int x=0; x < filterSize; x++)
	{
	    jstring jEkuOidFilter = (jstring) env->GetObjectArrayElement(jEkuOidFilters, x);

	    const char* pszEkuOidFilter = env->GetStringUTFChars(jEkuOidFilter, (boolean*)0);
	    strcat(szEkuOidFilters, pszEkuOidFilter);
	    
	    if (x != filterSize - 1)
		strcat(szEkuOidFilters, ",");

	    env->ReleaseStringUTFChars(jEkuOidFilter, pszEkuOidFilter);
	}	

	// Determine method ID to generate certificate
	jmethodID m = env->GetMethodID(env->GetObjectClass(obj), 
					"generateCertificate",
					"([BLjava/util/Collection;)V");

	// Use CertEnumCertificatesInStore to get the certificates 
	// from the open store. pCertContext must be reset to
	// NULL to retrieve the first certificate in the store.
	while (pCertContext= ::CertEnumCertificatesInStore(hCertStore, pCertContext))
	{
	    // Add certificate into collection only if key usage is valid for the filter
	    if (IsKeyUsageValid(pCertContext, szEkuOidFilters))
	    {
		BYTE* pbCertEncoded = pCertContext->pbCertEncoded;
		DWORD cbCertEncoded = pCertContext->cbCertEncoded;

		// Allocate and populate byte array
		jbyteArray byteArray = env->NewByteArray(cbCertEncoded);
		env->SetByteArrayRegion(byteArray, 0, cbCertEncoded, (jbyte*) pbCertEncoded);

		// Generate certificate from byte array and store into collection
		env->CallVoidMethod(obj, m, byteArray, jCertCollections);

		// Free byte array
		env->DeleteLocalRef(byteArray);
	    }
	}
    }
    __finally
    {
	if (hCertStore)
	    ::CertCloseStore(hCertStore, 0);

	if (pszCertStoreName)
	    env->ReleaseStringUTFChars(jCertStoreName, pszCertStoreName);
    }
}

/*
 * Class:     com_sun_deploy_security_WIExplorerMyKeyStore
 * Method:    loadKeysAndCertificateChains
 * Signature: (Ljava/lang/String;Ljava/util/Collection;)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_security_WIExplorerMyKeyStore_loadKeysAndCertificateChains
  (JNIEnv *env, jobject obj, jstring jCertStoreName, jobject jCollections)
{
    /**
     * Certificate stored in Internet Explorer cert store has enhanced key usage extension 
     * property (or EKU property) that is not part of the certificate itself. To determine
     * if the certificate should be returned, both the enhanced key usage in certificate 
     * extension block and the extension property stored along with the certificate in 
     * certificate store should be examined. Otherwise, we won't be able to determine
     * the proper key usage from the Java side because the information is not stored as
     * part of the encoded certificate.
     */

    const char* pszCertStoreName = NULL;
    HCERTSTORE hCertStore = NULL;        
    PCCERT_CONTEXT pCertContext=NULL; 

    __try
    {
	// Open a system certificate store.
        pszCertStoreName = env->GetStringUTFChars(jCertStoreName, (boolean*)0);
	if ((hCertStore = ::CertOpenSystemStore(NULL, pszCertStoreName)) == NULL)
	    __leave;

	// Determine clazz and method ID to generate certificate 
	jclass clazzArrayList = env->FindClass("java/util/ArrayList");

	jmethodID mNewArrayList = env->GetMethodID(clazzArrayList, "<init>", "()V");

	jmethodID mGenCert = env->GetMethodID(env->GetObjectClass(obj), 
					      "generateCertificate",
					      "([BLjava/util/Collection;)V");

	// Determine method ID to generate RSA certificate chain
	jmethodID mGenRSAKeyAndCertChain = env->GetMethodID(env->GetObjectClass(obj), 
						   "generateRSAKeyAndCertificateChain",
						   "(IIILjava/util/Collection;Ljava/util/Collection;)V");

	// Determine method ID to generate DSA certificate chain
	jmethodID mGenDSAKeyAndCertChain = env->GetMethodID(env->GetObjectClass(obj), 
						   "generateDSAKeyAndCertificateChain",
						   "(IIILjava/util/Collection;Ljava/util/Collection;)V");

	// Use CertEnumCertificatesInStore to get the certificates 
	// from the open store. pCertContext must be reset to
	// NULL to retrieve the first certificate in the store.
	while (pCertContext= ::CertEnumCertificatesInStore(hCertStore, pCertContext))
	{
	    // Check if private key available - client authentication certificate 
	    // must have private key available.
	    HCRYPTPROV hCryptProv = NULL;
	    DWORD dwKeySpec = 0;
	    BOOL bCallerFreeProv = FALSE;

	    if (::CryptAcquireCertificatePrivateKey(pCertContext, NULL, NULL, 
						    &hCryptProv, &dwKeySpec, &bCallerFreeProv) == FALSE)
	    {
		// Private key not found
		continue;
	    }

	    HCRYPTKEY hUserKey = NULL;
	    BOOL bGetUserKey = ::CryptGetUserKey(hCryptProv, dwKeySpec, &hUserKey);

	    // Skip certificate if cannot find private key
	    if (bGetUserKey == FALSE)
	    {
		if (bCallerFreeProv)
		    ::CryptReleaseContext(hCryptProv, NULL);			
			    
		continue;
	    }
	    
	    // Set cipher mode to ECB
	    DWORD dwCipherMode = CRYPT_MODE_ECB; 
	    ::CryptSetKeyParam(hUserKey, KP_MODE, (BYTE*)&dwCipherMode, NULL);


	    // If the private key is present in smart card, we may not be able to
	    // determine the key length by using the private key handle. However, 
	    // since public/private key pairs must have the same length, we could
	    // determine the key length of the private key by using the public key 
	    // in the certificate.  
	    DWORD dwPublicKeyLength = ::CertGetPublicKeyLength(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
							       &(pCertContext->pCertInfo->SubjectPublicKeyInfo));

	    PCCERT_CHAIN_CONTEXT pCertChainContext = NULL;

	    // Build certificate chain by using system certificate store. Add cert chain 
	    // into collection only if key usage is for the filter
	    //
	    if (GetCertificateChain(OID_EKU_CLIENT_AUTH, pCertContext, &pCertChainContext))
	    {
        	for (int i=0; i < pCertChainContext->cChain; i++)
		{
		    // Found cert chain
		    PCERT_SIMPLE_CHAIN rgpChain = pCertChainContext->rgpChain[i];

		    // Create ArrayList to store certs in each chain
		    jobject jArrayList = env->NewObject(clazzArrayList, mNewArrayList); 

		    for (int j=0; j < rgpChain->cElement; j++)
		    {
			PCERT_CHAIN_ELEMENT rgpElement = rgpChain->rgpElement[j];
			PCCERT_CONTEXT pc = rgpElement->pCertContext;

			BYTE* pbCertEncoded = pc->pbCertEncoded;
			DWORD cbCertEncoded = pc->cbCertEncoded;

			// Allocate and populate byte array
			jbyteArray byteArray = env->NewByteArray(cbCertEncoded);
			env->SetByteArrayRegion(byteArray, 0, cbCertEncoded, (jbyte*) pbCertEncoded);

			// Generate certificate from byte array and store into cert collection
			env->CallVoidMethod(obj, mGenCert, byteArray, jArrayList);
		    }

		    // Determine key type: RSA or DSA
		    DWORD dwData = CALG_RSA_KEYX;
		    DWORD dwSize = sizeof(DWORD);
		    ::CryptGetKeyParam(hUserKey, KP_ALGID, (BYTE*)&dwData, &dwSize, NULL);
		    
		    if ((dwData & ALG_TYPE_DSS) == ALG_TYPE_DSS)
		    {
			// Generate DSA certificate chain and store into cert chain collection
			env->CallVoidMethod(obj, mGenDSAKeyAndCertChain, hCryptProv, hUserKey, dwPublicKeyLength, jArrayList, jCollections);
		    }
		    else if ((dwData & ALG_TYPE_RSA) == ALG_TYPE_RSA)
		    {
			// Generate RSA certificate chain and store into cert chain collection
			env->CallVoidMethod(obj, mGenRSAKeyAndCertChain, hCryptProv, hUserKey, dwPublicKeyLength, jArrayList, jCollections);
		    }
		}

		// Free cert chain
		if (pCertChainContext)
		    ::CertFreeCertificateChain(pCertChainContext);
	    }
	}
    }
    __finally
    {
	if (hCertStore)
	    ::CertCloseStore(hCertStore, 0);

	if (pszCertStoreName)
	    env->ReleaseStringUTFChars(jCertStoreName, pszCertStoreName);
    }
}

/*
 * Class:     com_sun_deploy_security_MSCryptoRSAKey
 * Method:    cleanUp
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_security_MSCryptoRSAKey_cleanUp
  (JNIEnv *env, jclass clazz, jint hCryptProv, jint hCryptKey)
{
    if (hCryptKey != NULL)
	::CryptDestroyKey(hCryptKey);

    if (hCryptProv != NULL)	
	::CryptReleaseContext(hCryptProv, NULL);	
}


void ThrowSignatureException(JNIEnv *env, DWORD dwError)
{
    char szMessage[1024];
    szMessage[0] = '\0';

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, NULL, szMessage, 1024, NULL);	

    jclass exceptionClazz = env->FindClass("java/security/SignatureException");
    env->ThrowNew(exceptionClazz, szMessage);
}

/*
 * Class:     com_sun_deploy_security_MSCryptoNONEwithRSASignature
 * Method:    nativeSignHash
 * Signature: ([BIII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_sun_deploy_security_MSCryptoNONEwithRSASignature_nativeSignHash
  (JNIEnv *env, jclass clazz, jbyteArray jHash, jint jHashSize, jint hCryptProv, jint hCryptKey)
{
    HCRYPTHASH hHash = NULL;
    jbyte* pHashBuffer = NULL;
    jbyte* pSignedHashBuffer = NULL;
    jbyteArray jSignedHash = NULL;

    __try
    {
	// Acquire a hash object handle.
	if (::CryptCreateHash(HCRYPTPROV(hCryptProv), CALG_SSL3_SHAMD5, 0, 0, &hHash) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}
	
	// Copy hash from Java to native buffer
	pHashBuffer = new jbyte[jHashSize];
	env->GetByteArrayRegion(jHash, 0, jHashSize, pHashBuffer);	
	    
	// Set hash value in the hash object
	if (::CryptSetHashParam(hHash, HP_HASHVAL, (BYTE*)pHashBuffer, NULL) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}   
	
	// For RSA, the hash encryption algorithm is normally the same as the
	// public key algorithm, but it can be different too, we will call
	// CryptGetKeyParam API to decide which key spec to use.

	// Determine key spec.
	DWORD dwKeySpec = AT_SIGNATURE;
	ALG_ID dwAlgId;
	DWORD dwAlgIdLen = sizeof(ALG_ID);

	if (::CryptGetKeyParam((HCRYPTKEY) hCryptKey, KP_ALGID, (BYTE*)&dwAlgId, &dwAlgIdLen, 0) == FALSE) {
	    ThrowSignatureException(env, GetLastError());
 	    __leave;
	}

	if (CALG_RSA_KEYX == dwAlgId) {
	    dwKeySpec = AT_KEYEXCHANGE;
	}

	// Determine size of buffer
	DWORD dwBufLen = 0;
	if (::CryptSignHash(hHash, dwKeySpec, NULL, NULL, NULL, &dwBufLen) == FALSE) 
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}  
	
	pSignedHashBuffer = new jbyte[dwBufLen];
	if (::CryptSignHash(hHash, dwKeySpec, NULL, NULL, (BYTE*)pSignedHashBuffer, &dwBufLen) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}   

	// Create new byte array
	jbyteArray temp = env->NewByteArray(dwBufLen);
	
	// Copy data from native buffer
	env->SetByteArrayRegion(temp, 0, dwBufLen, pSignedHashBuffer);

	jSignedHash = temp;
    }
    __finally
    {
	if (pSignedHashBuffer)
	    delete [] pSignedHashBuffer;
	    
	if (pHashBuffer)
	    delete [] pHashBuffer;
	    
	if (hHash)
	    ::CryptDestroyHash(hHash);    
    }

    return jSignedHash;
}


/*	      
 * Class:     com_sun_deploy_security_MSCryptoDSASignature
 * Method:    nativeSignHash
 * Signature: ([BIII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_sun_deploy_security_MSCryptoDSASignature_nativeSignHash
  (JNIEnv *env, jclass clazz, jbyteArray jHash, jint jHashSize, jint hCryptProv, jint hCryptKey)
{
    HCRYPTHASH hHash = NULL;
    jbyte* pHashBuffer = NULL;
    jbyte* pSignedHashBuffer = NULL;
    jbyteArray jSignedHash = NULL;

    __try
    {
	// Acquire a hash object handle.
	if (::CryptCreateHash(HCRYPTPROV(hCryptProv), CALG_SHA1, 0, 0, &hHash) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}
	
	// Copy hash from Java to native buffer
	pHashBuffer = new jbyte[jHashSize];
	env->GetByteArrayRegion(jHash, 0, jHashSize, pHashBuffer);	
	    
	// Set hash value in the hash object
	if (::CryptSetHashParam(hHash, HP_HASHVAL, (BYTE*)pHashBuffer, NULL) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}   
	
	// For DSA, the hash encryption algorithm is normally a DSS 
	// signature algorithm, so AT_SIGNATURE is used.
	
	// Determine size of buffer
	DWORD dwBufLen = 0;
	if (::CryptSignHash(hHash, AT_SIGNATURE, NULL, NULL, NULL, &dwBufLen) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	 }  
	
	pSignedHashBuffer = new jbyte[dwBufLen];
    
	if (::CryptSignHash(hHash, AT_SIGNATURE, NULL, NULL, (BYTE*)pSignedHashBuffer, &dwBufLen) == FALSE)
	{
    	    ThrowSignatureException(env, GetLastError());
	    __leave;
	}   

	// Create new byte array
	jbyteArray temp = env->NewByteArray(dwBufLen);
	
	// Copy data from native buffer
	env->SetByteArrayRegion(temp, 0, dwBufLen, pSignedHashBuffer);

	jSignedHash = temp;
    }
    __finally
    {
	if (pSignedHashBuffer)
	    delete [] pSignedHashBuffer;
	    
	if (pHashBuffer)
	    delete [] pHashBuffer;
	    
	if (hHash)
	    ::CryptDestroyHash(hHash);    
    }

    return jSignedHash;
}

}
