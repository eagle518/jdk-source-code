/*
 * @(#)cookie.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
     char *szCookieInfo = NULL;
     DWORD dwSize = 0;
     char szCookieName[COOKIE_NAME_LENGTH];

     szCookieName[0]='\0';

     // Get cookie size into dwsize
     if(::InternetGetCookie(szURL, szCookieName, NULL, &dwSize)) {
       if (dwSize != 0) {
          // Get the cookie into szCookieInfo
          if ((szCookieInfo = (char *)malloc(sizeof(char) * dwSize)) != NULL) {
             ::InternetGetCookie(szURL, szCookieName, szCookieInfo, &dwSize);
          }
       }
     }

     if (szURL)
        env->ReleaseStringUTFChars(url, szURL);

     jstring ret = NULL;

     if (szCookieInfo != NULL) {
        ret = env->NewStringUTF(szCookieInfo);
        free(szCookieInfo);
     }

     return ret;
}

}
