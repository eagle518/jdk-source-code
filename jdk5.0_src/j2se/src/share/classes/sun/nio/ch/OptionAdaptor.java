/*
 * @(#)OptionAdaptor.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.io.*;
import java.net.*;
import java.nio.*;
import java.nio.channels.*;


// Adaptor class for java.net-style options
//
// The option get/set methods in the socket, server-socket, and datagram-socket
// adaptors delegate to an instance of this class.
// 

class OptionAdaptor {					// package-private

    private final SocketOpts.IP opts;

    OptionAdaptor(SocketChannelImpl sc) {
	opts = (SocketOpts.IP)sc.options();
    }

    OptionAdaptor(ServerSocketChannelImpl ssc) {
	opts = (SocketOpts.IP)ssc.options();
    }

    OptionAdaptor(DatagramChannelImpl dc) {
	opts = (SocketOpts.IP)dc.options();
    }

    private SocketOpts.IP opts() {
	return opts;
    }

    private SocketOpts.IP.TCP tcpOpts() {
	return (SocketOpts.IP.TCP)opts;
    }

    public void setTcpNoDelay(boolean on) throws SocketException {
	try {
	    tcpOpts().noDelay(on);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public boolean getTcpNoDelay() throws SocketException {
	try {
	    return tcpOpts().noDelay();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return false;		// Never happens
	}
    }

    public void setSoLinger(boolean on, int linger) throws SocketException {
	try {
	    if (linger > 65535)
		linger = 65535;
	    opts().linger(on ? linger : -1);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public int getSoLinger() throws SocketException {
	try {
	    return opts().linger();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return 0;			// Never happens
	}
    }

    public void setOOBInline(boolean on) throws SocketException {
	try {
	    opts().outOfBandInline(on);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public boolean getOOBInline() throws SocketException {
	try {
	    return opts().outOfBandInline();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return false;		// Never happens
	}
    }

    public void setSendBufferSize(int size)
	throws SocketException
    {
	try {
	    opts().sendBufferSize(size);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public int getSendBufferSize() throws SocketException {
	try {
	    return opts().sendBufferSize();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return 0;			// Never happens
	}
    }

    public void setReceiveBufferSize(int size)
	throws SocketException
    {
	try {
	    opts().receiveBufferSize(size);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public int getReceiveBufferSize() throws SocketException {
	try {
	    return opts().receiveBufferSize();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return 0;			// Never happens
	}
    }

    public void setKeepAlive(boolean on) throws SocketException {
	try {
	    opts().keepAlive(on);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public boolean getKeepAlive() throws SocketException {
	try {
	    return opts().keepAlive();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return false;		// Never happens
	}
    }

    public void setTrafficClass(int tc) throws SocketException {
	if (tc < 0 || tc > 255)
	    throw new IllegalArgumentException("tc is not in range 0 -- 255");
	try {
	    opts().typeOfService(tc);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public int getTrafficClass() throws SocketException {
	try {
	    return opts().typeOfService();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return 0;			// Never happens
	}
    }

    public void setReuseAddress(boolean on)
	throws SocketException
    {
	try {
	    opts().reuseAddress(on);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public boolean getReuseAddress() throws SocketException {
	try {
	    return opts().reuseAddress();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return false;		// Never happens
	}
    }

    public void setBroadcast(boolean on)
	throws SocketException
    {
	try {
	    opts().broadcast(on);
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	}
    }

    public boolean getBroadcast() throws SocketException {
	try {
	    return opts().broadcast();
	} catch (Exception x) {
	    Net.translateToSocketException(x);
	    return false;		// Never happens
	}
    }

}
