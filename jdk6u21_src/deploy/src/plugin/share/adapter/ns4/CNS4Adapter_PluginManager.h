/*
 * @(#)CNS4Adapter_PluginManager.h	1.7 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginManager.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginManager.h: interface for the CNS4Adapter_PluginManager class.
//
// This is the dummy plugin manager that interacts with the 5.0 plugin.
//

#if !defined(AFX_CNS4ADAPTER_PLUGINMANAGER_H__3651882A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
#define AFX_CNS4ADAPTER_PLUGINMANAGER_H__3651882A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPluginManager.h"

class INS4AdapterPeer;

class CNS4Adapter_PluginManager : public IPluginManager 
{
public:

    CNS4Adapter_PluginManager(INS4AdapterPeer* peer);
    virtual ~CNS4Adapter_PluginManager(void);

    JD_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////////
    // from IPluginManager:
    // (Corresponds to NPN_UserAgent.)
    JD_IMETHOD
    UserAgent(const char* *result);

    JD_IMETHOD 
    GetValue(JDPluginManagerVariable variable, void* value);
    
    JD_IMETHOD
    GetURL(ISupports* pluginInst, 
           const char* url, 
           const char* target = NULL,
           IPluginStreamListener* streamListener = NULL,
           const char* altHost = NULL,
           const char* referrer = NULL,
           JDBool forceJSEnabled = JD_FALSE);
    
    JD_IMETHOD
    FindProxyForURL(const char* url, char* *result);
   
private:
    INS4AdapterPeer* m_pINS4AdapterPeer;
};



#endif // !defined(AFX_CNS4ADAPTER_PLUGINMANAGER_H__3651882A_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
