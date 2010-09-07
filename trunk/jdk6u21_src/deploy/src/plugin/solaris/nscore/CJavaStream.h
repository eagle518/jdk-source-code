/*
 * @(#)CJavaStream.h	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 *  Taken from Stanley Ho's Windows version.
 * 
 */

#ifndef CJAVASTREAM_H
#define CJAVASTREAM_H

#define MAX_PATH 4096

#include "IPluginInstance.h"
#include "IPluginStreamInfo.h"
#include "CNetscapeStream.h"

class CJavaStream : public CNetscapeStream 
{

public:
    CJavaStream(IPluginInstance* pPlugin, IPluginStreamInfo* stream);

    virtual ~CJavaStream();

    // Handle a piece of data in the stream
    JD_IMETHOD Write(const char* buffer, JDint32 offset, JDint32 len, JDint32 *bytesWritten);
    
    // (Corresponds to NPP_NewStream's stype return parameter.)
    JD_IMETHOD GetStreamType(JDPluginStreamType *result);

    JD_IMETHOD AsFile(const char *fname);

protected:
    // Plugin factory object
    IPluginInstance*  m_PluginInstance;		

    // Number of times write is called
    long  m_iCount;

    // URL of the stream data
    char* m_pszURL;	
};

#endif
