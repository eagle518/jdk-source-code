/*
 * @(#)CNetscapeStream.cpp	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// CNetscapeStream.cpp  by Stanley Man-Kit Ho
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Contains implementation of the CNetscapeStream
//

#include <stdio.h>
#include <stdlib.h>
#include "IPluginStream.h"
#include "CNetscapeStream.h"

const long CNetscapeStream::kMaxBufSize = 0X0FFFFFFF;
#define UNUSED(x) x=x
//======================================================================
//		CNetscapeStream::CNetscapeStream
//======================================================================

CNetscapeStream::CNetscapeStream(IPluginStreamInfo* stream)
:	mStreamPeer( stream )
{

	JD_INIT_REFCNT();

	mStreamPeer->AddRef();
}

//======================================================================
//		CNetscapeStream::~CNetscapeStream
//======================================================================

CNetscapeStream::~CNetscapeStream()
{

	if (mStreamPeer != NULL)
		mStreamPeer->Release();
}


JD_IMPL_ISUPPORTS1(CNetscapeStream, IPluginStream);

//======================================================================
//		CNetscapeStream::Write
//======================================================================
JD_IMETHODIMP
CNetscapeStream::Write(const char* buffer, JDint32 offset, JDint32 len,
		       JDint32 *bytesWritten)
{
	*bytesWritten = len;
        UNUSED(buffer);
	return JD_OK;
}

JD_IMETHODIMP CNetscapeStream::Close(void)
{
    return JD_OK;
}

//======================================================================
//		CNetscapeStream::GetStreamType
//======================================================================

// (Corresponds to NPP_NewStream's stype return parameter.)
JD_IMETHODIMP
CNetscapeStream::GetStreamType(JDPluginStreamType *result)
{
    *result =  JDPluginStreamType_Normal;
    return JD_OK;
}

//======================================================================
//		CNetscapeStream::CNetscapeStream
//======================================================================

// (Corresponds to NPP_StreamAsFile.)
JD_IMETHODIMP
CNetscapeStream::AsFile(const char* fname)
{ 
    return JD_OK;
}
