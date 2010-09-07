/*
 * @(#)CNS4Adapter_PluginInputStream.cpp	1.11 03/01/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginInputStream.cpp    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginInputStream.cpp: implementation for the 
//				     CNS4Adapter_PluginInputStream class.
//
// This is the dummy stream peer that interacts with the 5.0 plugin.
//

#include "StdAfx.h"
#include "npapi.h"
#include "IPluginStreamInfo.h"
#include "IPluginStreamListener.h"
#include "INS4AdapterPeer.h"
#include "CNS4Adapter_PluginStreamInfo.h"
#include "Debug.h"
//=--------------------------------------------------------------------------=
// Global variables
//=--------------------------------------------------------------------------=
extern "C" JDresult JDResultFromNPError(int err);


//////////////////////////////////////////////////////////////////////////////

JD_IMPL_ISUPPORTS1(CNS4Adapter_PluginStreamInfo, IPluginStreamInfo);


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::CNS4Adapter_PluginStreamInfo
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginStreamInfo::CNS4Adapter_PluginStreamInfo(INS4AdapterPeer* peer, 
							   NPP instance, 
							   NPStream* stream, 
							   JDPluginMimeType mimeType, 
							   JDBool seekable)
    : mNPP(instance), mStream(stream), mURL(NULL),
      mMimeType(NULL), mSeekable(seekable), m_pINS4AdapterPeer(NULL)
{
    TRACE("CNS4Adapter_PluginStreamInfo::CNS4Adapter_PluginStreamInfo\n");

    ASSERT(peer != NULL);

    JD_INIT_REFCNT();

    // Make sure MIME type has been copied!!
    if (mimeType != NULL)
    {
	char* pMimeType = new char[strlen(mimeType) + 1];

	if (pMimeType != NULL)
	    strcpy(pMimeType, mimeType);

	mMimeType = pMimeType;
    }

    // Make sure URL has been copied!!
    if (stream->url != NULL)
    {
	char* pURL = new char[strlen(stream->url) + 1];

	if (mURL != NULL)
	    strcpy(pURL, stream->url);

	mURL = pURL;
    }

    m_pINS4AdapterPeer = peer;

    if (m_pINS4AdapterPeer != NULL)
	m_pINS4AdapterPeer->AddRef();
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::~CNS4Adapter_PluginStreamInfo
//=--------------------------------------------------------------------------=
//
CNS4Adapter_PluginStreamInfo::~CNS4Adapter_PluginStreamInfo(void)
{
    TRACE("CNS4Adapter_PluginStreamInfo::~CNS4Adapter_PluginStreamInfo\n");

    ASSERT(m_pINS4AdapterPeer != NULL);

    if (mMimeType != NULL)
    {
    	delete [] ((char*) mMimeType);
    }

    if (mURL != NULL)
    {
    	delete [] ((char*) mURL);
    }

    if (m_pINS4AdapterPeer != NULL)
	m_pINS4AdapterPeer->Release();
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::GetURL
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::GetURL(const char** result)
{
    TRACE("CNS4Adapter_PluginStreamInfo::GetURL\n");

    ASSERT(mStream != NULL);
    ASSERT(result != NULL);

    if (mNPP == NULL || mStream == NULL || result == NULL)
        return JD_ERROR_FAILURE;

    *result = mURL;
    return JD_OK;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::GetContentType
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::GetContentType(JDPluginMimeType* result)
{
    TRACE("CNS4Adapter_PluginStreamInfo::GetContentType\n");

    ASSERT(result != NULL);

    if (mNPP == NULL || mStream == NULL || result == NULL)
        return JD_ERROR_FAILURE;

    *result = mMimeType;
    return JD_OK;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::IsSeekable
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::IsSeekable(JDBool* result)
{
    TRACE("CNS4Adapter_PluginStreamInfo::IsSeekable\n");

    ASSERT(result != NULL);

    if (mNPP == NULL || mStream == NULL || result == NULL)
        return JD_ERROR_FAILURE;

    *result = mSeekable;
    return JD_OK;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::GetLength
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::GetLength(JDUint32* result)
{
    TRACE("CNS4Adapter_PluginStreamInfo::GetLength\n");

    ASSERT(mStream != NULL);
    ASSERT(result != NULL);

    if (mNPP == NULL || mStream == NULL || result == NULL)
        return JD_ERROR_FAILURE;

    *result = mStream->end;
    return JD_OK;
}


//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::GetLastModified
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::GetLastModified(JDUint32 *result)
{
    TRACE("CNS4Adapter_PluginStreamInfo::GetLastModified\n");

    ASSERT(mStream != NULL);
    ASSERT(result != NULL);

    if (mNPP == NULL || mStream == NULL || result == NULL)
        return JD_ERROR_FAILURE;

    *result = mStream->lastmodified;
    return JD_OK;
}

//=--------------------------------------------------------------------------=
// CNS4Adapter_PluginStreamInfo::GetNotifyData
//=--------------------------------------------------------------------------=
//
JD_METHOD
CNS4Adapter_PluginStreamInfo::GetNotifyData(void* *result) 
{
  TRACE("CNS4Adapter_PluginStreamInfo::GetNotifyData\n");

  *result = mStream->notifyData;
  return JD_OK;
}
