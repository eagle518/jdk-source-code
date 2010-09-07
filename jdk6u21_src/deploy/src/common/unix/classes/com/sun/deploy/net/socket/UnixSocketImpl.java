/*
 * @(#)UnixSocketImpl.java	1.3 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.socket;

public class UnixSocketImpl
{
    protected static boolean unStreamSocketSupported()
    {
        return true;
    }

    protected static native long unStreamSocketCreate(String fileName, boolean abstractNamespace, int protocol) 
        throws UnixDomainSocketException;

    protected static native void unStreamSocketClose(long unSocketHandle)
        throws UnixDomainSocketException;

    protected static native boolean unStreamSocketIsValid(long unSocketHandle)
        throws UnixDomainSocketException;

    protected static native void unStreamSocketBind(long unSocketHandle)
        throws UnixDomainSocketException;

    protected static native void unStreamSocketListen(long unSocketHandle, int backlog)
        throws UnixDomainSocketException;

    protected static native long unStreamSocketAccept(long unSocketHandleServer)
        throws UnixDomainSocketException;

    protected static native void unStreamSocketConnect(long unSocketHandle)
        throws UnixDomainSocketException;

    protected static native int unStreamSocketRead(long unSocketHandle, Object buffer, int offset, int count)
        throws UnixDomainSocketException;

    protected static native int unStreamSocketWrite(long unSocketHandle, Object buffer, int offset, int count)
        throws UnixDomainSocketException;

    protected static native String unStreamSocketGetNativeInfo(long unSocketHandle);
}

