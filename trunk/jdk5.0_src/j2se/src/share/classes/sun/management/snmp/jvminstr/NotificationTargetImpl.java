/*
 * @(#)file      NotificationTargetImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.4
 * @(#)lastedit  03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.jvminstr;

import java.net.InetAddress;
import java.net.UnknownHostException;

/**
 * Notification Target data.
 */
public class NotificationTargetImpl implements NotificationTarget {
    private InetAddress address;
    private int port;
    private String community;
    
    /**
     * Target parameter is a <CODE>java.lang.String</CODE>
     * target synctax is <host>:<port>:community. Eg: "localhost:163:private".
     * <p>The <code><em>host</em></code> is a host name, an IPv4 numeric
     * host address, or an IPv6 numeric address enclosed in square
     * brackets.</p>
     * @throws IllegalArgumentException In case the target is malformed
     */
    public NotificationTargetImpl(String target) 
	throws IllegalArgumentException, UnknownHostException  {
	parseTarget(target);
    }
    
    public NotificationTargetImpl(String address, int port, 
				  String community) 
	throws UnknownHostException {
	this(InetAddress.getByName(address),port,community);
    }
    
    public NotificationTargetImpl(InetAddress address, int port, 
				  String community) {
	this.address = address;
	this.port = port;
	this.community = community;
    }
    
    private void parseTarget(String target) 
	throws IllegalArgumentException, UnknownHostException {
	
	if(target == null || 
	   target.length() == 0) throw new 
	       IllegalArgumentException("Invalid target [" + target + "]");
	
	String addrStr;
	if (target.startsWith("[")) {
	    final int index = target.indexOf("]");
	    final int index2 = target.lastIndexOf(":");
	    if(index == -1)
		throw new IllegalArgumentException("Host starts with [ but " +
						   "does not end with ]");
	    addrStr = target.substring(1, index);
	    port = Integer.parseInt(target.substring(index + 2, 
						     index2));
	    if (!isNumericIPv6Address(addrStr)) {
	    throw new IllegalArgumentException("Address inside [...] must " +
					       "be numeric IPv6 address");
	    }
	    if (addrStr.startsWith("["))
		throw new IllegalArgumentException("More than one [[...]]");   
	} else {
	    final int index = target.indexOf(":");
	    final int index2 = target.lastIndexOf(":");
	    if(index == -1) throw new 
		IllegalArgumentException("Missing port separator \":\"");
	    addrStr = target.substring(0, index);
	    
	    port = Integer.parseInt(target.substring(index + 1,
						     index2));
	}

	address = InetAddress.getByName(addrStr);

	//THE CHECK SHOULD BE STRONGER!!!
	final int index = target.lastIndexOf(":");
	
	community = target.substring(index + 1, target.length());
	
    }
    
    /* True if this string, assumed to be a valid argument to
     * InetAddress.getByName, is a numeric IPv6 address.
     */
    private static boolean isNumericIPv6Address(String s) {
	// address contains colon iff it's a numeric IPv6 address
	return (s.indexOf(':') >= 0);
    }

    public String getCommunity() {
	return community;
    }
    
    public InetAddress getAddress() {
	return address;
    }

    public int getPort() {
	return port;
    }

    public String toString() {
	return "address : " + address + " port : " + port + 
	    " community : " + community;
    }
}
