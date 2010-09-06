/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)CNetscapeStream.h	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// CNetscapeStream.h  by Stanley Man-Kit Ho
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
// Contains declaration of the CNetscapeStream
//

#ifndef CNETSCAPESTREAM_H
#define CNETSCAPESTREAM_H

#include "IPluginStream.h"
#include "IPluginStreamInfo.h"

class CNetscapeStream : public IPluginStream
{
public:
    CNetscapeStream(IPluginStreamInfo* peer);
    virtual	~CNetscapeStream();
		
    //================================================================
    // nsISupport
    //================================================================
    JD_DECL_ISUPPORTS		

    //================================================================
    // nsIStream
    //================================================================

    JD_METHOD Close(void);

    // (Corresponds to nsP_Write and nsN_Write.)
    JD_IMETHOD Write(const char* buffer, JDint32 offset, JDint32 len, JDint32 *bytesWritten);

    //================================================================
    // nsIPluginStream
    //================================================================

    // (Corresponds to nsP_NewStream's stype return parameter.)
    JD_IMETHOD GetStreamType(JDPluginStreamType *result);

    // (Corresponds to nsP_StreamAsFile.)
    JD_IMETHOD AsFile(const char* fname);		
protected:
    static const long	 kMaxBufSize;

    IPluginStreamInfo* mStreamPeer;
};

#endif
