/*
  * @(#)CNSAdapter_PluginManager.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginManager.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_PluginManager.h is Declaration of adapter for nsIPluginManager
// 
//
#ifndef _CNSAdapter_PluginManager_h__
#define _CNSAdapter_PluginManager_h__

#include "IPluginManager.h"
#include "ICookieStorage.h"

class nsIPluginManager;
class IPluginStreamListener;

class CNSAdapter_PluginManager : public IPluginManager, 
				 public ICookieStorage
{
public:
    CNSAdapter_PluginManager(nsIPluginManager* pPluginManager);
    virtual ~CNSAdapter_PluginManager();

    // ISupports
    JD_DECL_ISUPPORTS

    // IPluginManager
    JD_IMETHOD 
    GetValue(JDPluginManagerVariable variable, void* value);
   
    JD_IMETHOD
    UserAgent(const char * * resultingAgentString);

    JD_IMETHOD
    GetURL(ISupports* pluginInst,
           const char* url,
           const char* target = NULL,
           IPluginStreamListener* sl = NULL,
	   const char* altHost = NULL,
           const char* referrer = NULL,
           JDBool forceJSEnabled = PR_FALSE);

    JD_IMETHOD
    FindProxyForURL(const char* url, char* *result);

    // ICookieStorage
    JD_IMETHOD
    GetCookie(const char* inCookieURL, void* inOutCookieBuffer, JDUint32& inOutCookieSize);

    JD_IMETHOD
    SetCookie(const char* inCookieURL, const void* inCookieBuffer, JDUint32 inCookieSize);

private:
    nsIPluginManager* m_pPluginManager;
};

#endif //_CNSAdapter_PluginManager_h__
