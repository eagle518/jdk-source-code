/*
 * @(#)IInputStream.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IInputStream.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for I/O
//
#ifndef _IINPUTSTREAM_H_
#define _IINPUTSTREAM_H_

#include "ISupports.h"
/*7A168FD7-A576-11d6-9A82-00B0D0A18D51*/
#define IINPUTSTREAM_IID		\
{ 0x7A168FD7, 0xa576, 0x11d6, {0x9A, 0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} }

class IInputStream : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IINPUTSTREAM_IID);
    
    /**
     * Close the stream 
     */
    JD_IMETHOD Close(void) = 0;

    /**
     * @return number of bytes currently available in the stream
     */
    JD_IMETHOD Available(JDUint32 *_retval) = 0;

    /** 
     * Read data from the stream.
     *
     * @param aBuf the buffer into which the data is to be read
     * @param aCount the maximum number of bytes to be read
     *
     * @return number of bytes read
     * @return 0 if reached end of file
     *
     * @throws NS_BASE_STREAM_WOULD_BLOCK if reading from the input stream would
     *   block the calling thread (non-blocking mode only)
     * @throws <other-error> on failure
     */
    JD_IMETHOD Read(char * buf, JDUint32 count, JDUint32 *_retval) = 0;
};

#endif //_IINPUTSTREAM_H_
