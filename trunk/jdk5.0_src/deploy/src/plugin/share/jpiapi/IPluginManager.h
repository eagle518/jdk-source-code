/*
 * @(#)IPluginManager.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPluginManager.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
#ifndef _IPLUGINMANAGER_H_
#define _IPLUGINMANAGER_H_

#include "ISupports.h"
#include "IPluginStreamListener.h"


//{3BB20CB1-9B7D-11d6-9A7D-00B0D0A18D51}
#define CPLUGINMANAGER_CID \
{0x3BB20CB1, 0x9B7D, 0x11d6, {0x9A, 0x7D, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//{EFD74BDD-99B7-11d6-9A76-00B0D0A18D51}
#define IPLUGINMANAGER_IID \
{0xEFD74BDD, 0x99B7, 0x11d6, {0x9A, 0x76, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//ISupports interface (A replicate of nsISupports)
class IPluginManager : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINMANAGER_IID);
	
    JD_DEFINE_STATIC_CID_ACCESSOR(CPLUGINMANAGER_CID);

    JD_IMETHOD GetValue(JDPluginManagerVariable variable, void* value) = 0;
    
    JD_IMETHOD UserAgent(const char * * resultingAgentString) = 0;

    JD_IMETHOD
    GetURL(ISupports* pluginInst,
           const char* url,
           const char* target = NULL,
           IPluginStreamListener* sl = NULL,
           const char* altHost = NULL,
           const char* referrer = NULL,
           JDBool forceJSEnabled = JD_FALSE) = 0;

    JD_IMETHOD
    FindProxyForURL(const char* url, char* *result) = 0;
};

#endif // _IPLUGINMANAGER_H_
