/*
 * @(#)UnixSocketImpl.java	1.3 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.socket;

import java.lang.UnsupportedOperationException;

public class UnixSocketImpl
{
    protected static boolean unStreamSocketSupported()
    {
        return false;
    }

    protected static long unStreamSocketCreate(String fileName, boolean abstractNamespace, int protocol) 
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static void unStreamSocketClose(long unSocketHandle)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static boolean unStreamSocketIsValid(long unSocketHandle)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static void unStreamSocketBind(long unSocketHandle)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static void unStreamSocketListen(long unSocketHandle, int backlog)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static long unStreamSocketAccept(long unSocketHandleServer)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static void unStreamSocketConnect(long unSocketHandle)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static int unStreamSocketRead(long unSocketHandle, Object buffer, int offset, int count)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static int unStreamSocketWrite(long unSocketHandle, Object buffer, int offset, int count)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

    protected static String unStreamSocketGetNativeInfo(long unSocketHandle)
        throws UnsupportedOperationException
    {
        throw new UnsupportedOperationException("not supported on windows");
    }

}

