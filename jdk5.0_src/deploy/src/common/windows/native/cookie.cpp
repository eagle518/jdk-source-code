/*
 * @(#)cookie.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// security.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <windows.h>
#include <wincrypt.h>
#include <wininet.h>
#include <jni.h>


// Cookie size should not exceed 4096 chars + 1 for '\0'
// according to cookie spec.
#define COOKIE_BUFFER_LENGTH 4097

// Cookie name should not exceed 10 chars + 1 for '\0'
// according to cookie spec.
#define COOKIE_NAME_LENGTH 11

/////////////////////////////////////////////////////////////////////////////
//

extern "C" {

/*
 * Class:     com_sun_deploy_net_cookie_IExplorerCookieHandler
 * Method:    setCookieInfo
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_net_cookie_IExplorerCookieHandler_setCookieInfo
  (JNIEnv *env, jobject, jstring url, jstring value)
{
    if (url == NULL || value == NULL)
	return;


    const char* szURL = env->GetStringUTFChars(url, (jboolean*)0);    
    const char* szValue = env->GetStringUTFChars(value, (jboolean*)0);    

    // Set Cookie setting for a particular URL
    ::InternetSetCookie(szURL, NULL, szValue);

    if (szURL)
	env->ReleaseStringUTFChars(url, szURL);

    if (szValue)
	env->ReleaseStringUTFChars(value, szValue);
}


/*
!  * Class:     com_sun_deploy_net_cookie_IExplorerCookieHandler
!  * Method:    getCookieInfo
!  * Signature: (Ljava/lang/String;)Ljava/lang/String;
   */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_cookie_IExplorerCookieHandler_getCookieInfo
   (JNIEnv *env, jobject sender, jstring url)
{
     const char* szURL = env->GetStringUTFChars(url, (jboolean*)0);    
  
     // Resolve Cookie setting for a particular URL
     char szCookieInfo[COOKIE_BUFFER_LENGTH];
     DWORD dwSize = COOKIE_BUFFER_LENGTH;
 
     szCookieInfo[0] = '\0';
     
     char szCookieName[COOKIE_NAME_LENGTH];
     szCookieName[0]='\0';
     ::InternetGetCookie(szURL, szCookieName, szCookieInfo, &dwSize);
 
     if (szURL)
	env->ReleaseStringUTFChars(url, szURL);
 
     jstring ret = NULL;
 
     if (szCookieInfo[0] != '\0')
	ret = env->NewStringUTF(szCookieInfo);
 
     return ret;
}


}

