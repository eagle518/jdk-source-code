/*
 *  @(#)CNS4Adapter_PluginInputStream.h	1.9 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInputStream.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginInputStream.h: interface for the 
//				   CNS4Adapter_PluginInputStream class.
//
// This is the dummy stream peer that interacts with the 5.0 plugin.
//

#if !defined(AFX_CNS4ADAPTER_PLUGININPUTSTREAM_H__3651882D_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)
#define AFX_CNS4ADAPTER_PLUGININPUTSTREAM_H__3651882D_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPluginStreamInfo.h"
#include "INS4AdapterPeer.h"

class IPluginStreamListener;

class CNS4Adapter_PluginStreamInfo : public IPluginStreamInfo {
public:

    JD_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////////
    // from IPluginStreamInfo:

    // Corresponds to NPStream's url field.
    JD_IMETHOD
    GetURL(const char*  *result);

    JD_IMETHOD
    GetContentType(JDPluginMimeType* result);

    JD_IMETHOD
    IsSeekable(JDBool* result);

    JD_IMETHOD
    GetLength(JDUint32* result);

    JD_IMETHOD
    GetLastModified(JDUint32* result);
    
    JD_IMETHOD
      GetNotifyData(void* *result); 
	
    ////////////////////////////////////////////////////////////////////////////
    // CNS4Adapter_PluginStreamInfo specific methods:

    CNS4Adapter_PluginStreamInfo(INS4AdapterPeer* peer, NPP instance, 
				 NPStream* stream, JDPluginMimeType mimeType, 
				 JDBool seekable);
    virtual ~CNS4Adapter_PluginStreamInfo(void);

protected:
    INS4AdapterPeer* m_pINS4AdapterPeer;

    NPP		mNPP;
    NPStream*	mStream;
    char*	mURL;
    JDPluginMimeType	mMimeType;
    JDBool	mSeekable;
};


#endif // !defined(AFX_CNS4ADAPTER_PLUGININPUTSTREAM_H__3651882D_B7AE_11D2_BA19_00105A1F1DAB__INCLUDED_)


