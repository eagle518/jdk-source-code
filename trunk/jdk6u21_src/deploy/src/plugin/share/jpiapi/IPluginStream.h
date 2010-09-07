/*
 * @(#)IPluginStream.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPluginStream.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
// 
//
#ifndef _IPLUGININSTREAM_H_
#define _IPLUGININSTREAM_H_

#include "ISupports.h"

// {904325E8-A469-41d9-B777-71F44379C39C}
#define IPLUGINSTREAM_IID                               \
{0x904325e8, 0xa469, 0x41d9, {0xb7, 0x77, 0x71, 0xf4, 0x43, 0x79, 0xc3, 0x9c}}

class IPluginStream : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINSTREAM_IID);

    /** Write data into the stream.
     *  @param aBuf the buffer into which the data is read
     *  @param aOffset the start offset of the data
     *  @param aCount the maximum number of bytes to read
     *  @param aWriteCount out parameter to hold the number of
     *         bytes written. if an error occurs, the writecount
     *         is undefined
     *  @return error status
     */   
    JD_IMETHOD
    Write(const char* aBuf, JDint32 aOffset, JDint32 aCount, JDint32 *aWriteCount) = 0; 

    /** Close the stream. */
    JD_IMETHOD
    Close(void) = 0;

    /**
     * Returns the stream type of a stream. 
     *
     * (Corresponds to NPP_NewStream's stype return parameter.)
     *
     * @param result - the resulting stream type
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetStreamType(JDPluginStreamType *result) = 0;

    /**
     * This operation passes to the plugin the name of the file which
     * contains the stream data.
     *
     * (Corresponds to NPP_StreamAsFile.)
     *
     * @param fname - the file name
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    AsFile(const char* fname) = 0;
    
};

////////////////////////////////////////////////////////////////////////////////

#endif /* _IPluginStream_h___ */
