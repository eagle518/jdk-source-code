/*
 *  @(#)ProxyHelper.java	1.2 10/03/24
 * 
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.message.helper;

import java.io.IOException;
import java.net.*;
import sun.plugin2.message.Serializer;

/*
 * This class mirros the java.net.Proxy object so that the Proxy 
 * object can be transferred between processes.
 */

public final class ProxyHelper {

    private static final int DIRECT_PROXY = 0;
    private static final int HTTP_PROXY   = 1;
    private static final int SOCKS_PROXY  = 2;
    
    /* This helper method converts a given Proxy.Type to a integer. */
    private static int getProxyTypeID(Proxy.Type type) 
        throws IllegalArgumentException {    
        if (type == Proxy.Type.DIRECT) {
            return DIRECT_PROXY;
        } else if (type == Proxy.Type.HTTP) {
            return HTTP_PROXY;
        } else if (type == Proxy.Type.SOCKS) {
            return SOCKS_PROXY;
        } else {
            throw new IllegalArgumentException();
        }
    }

    /* This helper method converts a integer to a Proxy.Type. */
    private static Proxy.Type getProxyType(int id) 
        throws IllegalArgumentException {
        switch(id) {
        case DIRECT_PROXY:
            return Proxy.Type.DIRECT;
        case HTTP_PROXY:
            return Proxy.Type.HTTP;
        case SOCKS_PROXY:
            return Proxy.Type.SOCKS;
        default:
            throw new IllegalArgumentException();
        }
    }

    /** Writes the given ProxyObject to the given Serializer. */
    public static void write(Serializer ser, Proxy proxy) throws IOException {
        if (proxy == null) {
            ser.writeBoolean(false);
            return;
        }
       
        ser.writeBoolean(true);
        ser.writeInt(getProxyTypeID(proxy.type()));
        InetSocketAddress addr = (InetSocketAddress)proxy.address();
        if (addr != null) {
            ser.writeBoolean(true);
            ser.writeUTF(addr.getHostName());
            ser.writeInt(addr.getPort());
        } else {
            ser.writeBoolean(false);
        }        
    }

    /** Reads a URLObject from the given Serializer. */
    public static Proxy read(Serializer ser) throws IOException {
        if (!ser.readBoolean()) {
            return null;
        }
        
        Proxy.Type proxyType = getProxyType(ser.readInt());
        InetSocketAddress addr = null;
        if (ser.readBoolean()) {
            addr = new InetSocketAddress(ser.readUTF(),
                                         ser.readInt());
        }
          
        if (proxyType == Proxy.Type.DIRECT) {
            return Proxy.NO_PROXY;
        }
        
        return new Proxy(proxyType, addr);
    }
}
