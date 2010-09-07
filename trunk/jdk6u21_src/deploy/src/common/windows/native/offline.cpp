/*
 * @(#)offline.cpp	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// offline.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//

#include "stdafx.h"
#include <comdef.h>	   // COM smart pointer
#include <jni.h>
#include <Wininet.h>


extern "C"
{

/*
 * Class:     com_sun_deploy_net_offline_WIExplorerOfflineHandler
 * Method:    isGlobalOffline
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_net_offline_WIExplorerOfflineHandler_isGlobalOffline
  (JNIEnv *env, jobject)
{
    DWORD dwState = 0; 
    DWORD dwSize = sizeof(DWORD);
    BOOL bRet = FALSE;

    if (::InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            bRet = TRUE;
    }
    return bRet; 
}

/*
 * Class:     com_sun_deploy_net_offline_WIExplorerOfflineHandler
 * Method:    setGlobalOffline
 * Signature: (Z)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_net_offline_WIExplorerOfflineHandler_setGlobalOffline
  (JNIEnv *env, jobject, jboolean offline)
{
    INTERNET_CONNECTED_INFO ci;
    memset(&ci, 0, sizeof(ci));
    
    if (offline) 
    {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } 
    else 
    {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    ::InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));

    return TRUE;
}

/*
 * Class:     com_sun_deploy_net_offline_WIExplorerOfflineHandler
 * Method:    askUserGoOnline
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_net_offline_WIExplorerOfflineHandler_askUserGoOnline
  (JNIEnv *env, jobject, jstring jURL)
{
    BOOL bRet = FALSE;
    
    const char *szURL = env->GetStringUTFChars(jURL, NULL);

    // If globally offline, ask user for permission to go online.
    bRet = ::InternetGoOnline((LPTSTR) szURL, NULL, NULL);

    env->ReleaseStringUTFChars(jURL, szURL);
    
    return bRet;
}

}

