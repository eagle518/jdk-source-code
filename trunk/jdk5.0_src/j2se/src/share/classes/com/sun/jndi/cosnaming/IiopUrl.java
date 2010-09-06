/*
 * @(#)IiopUrl.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.cosnaming;

import javax.naming.Name;
import javax.naming.NamingException;

import java.net.MalformedURLException;
import java.util.Vector;
import java.util.StringTokenizer;
import com.sun.jndi.toolkit.url.UrlUtil;

/**
 * Extract components of an "iiop" or "iiopname" URL.
 *
 * The format of an iiopname URL is defined in INS 98-10-11 as follows:
 *
 * iiopname url	= "iiopname://" [addr_list]["/" string_name]
 * addr_list 	= [address ","]* address
 * address 	= [version host [":" port]]
 * host		= DNS style host name | IP address
 * version	= major "." minor "@" | empty_string
 * port		= number
 * major	= number
 * minor	= number
 * string_name = stringified name | empty_string
 *
 * The default port is 9999. The default version is "1.0"
 * US-ASCII alphanumeric characters are not escaped. Any characters outside 
 * of this range are escaped except for the following:
 * ; / : ? : @ & = + $ , - _ . ! ~ *  ' ( )
 * Escaped characters is escaped by using a % followed by its 2 hexadecimal
 * numbers representing the octet.
 *
 * For backward compatibility,  the "iiop" URL as defined in INS 97-6-6
 * is also supported:
 * 
 * iiop url 	= "iiop://" [host [":" port]] ["/" string_name]
 * The default port is 900.
 *
 * @author Rosanna Lee
 * @version 1.7 03/12/19
 */

public final class IiopUrl {
    static final private int DEFAULT_IIOPNAME_PORT = 9999;
    static final private int DEFAULT_IIOP_PORT = 900;
    static final private String DEFAULT_HOST = "localhost";
    private Vector addresses;
    private String stringName;

    public static class Address {
	public int port, major, minor;
	public String host;

	public Address(String hostPortVers, boolean oldFormat) 
	    throws MalformedURLException {
	    // [version host [":" port]]
	    int start;

	    // Parse version
	    int at;
	    if (oldFormat || (at = hostPortVers.indexOf('@')) < 0) {
		major = 1;
		minor = 0;
		start = 0;     // start at the beginning
	    } else {
		int dot = hostPortVers.indexOf('.');
		if (dot < 0) {
		    throw new MalformedURLException(
			"invalid version: " + hostPortVers);
		}
		try {
		    major = Integer.parseInt(hostPortVers.substring(0, dot));
		    minor = Integer.parseInt(hostPortVers.substring(dot+1, at));
		} catch (NumberFormatException e) {
		    throw new MalformedURLException(
			"Nonnumeric version: " + hostPortVers);
		}
		start = at + 1;  // skip '@' sign
	    }

	    // Parse host/port
	    int colon = hostPortVers.indexOf(':', start);
	    if (colon < 0) {
		port = (oldFormat ? DEFAULT_IIOP_PORT : DEFAULT_IIOPNAME_PORT);
		host = hostPortVers.substring(start);
	    } else {
		try {
		    port = Integer.parseInt(hostPortVers.substring(colon + 1));
		} catch (NumberFormatException e) {
		    throw new MalformedURLException(
			"Nonnumeric port number: " + hostPortVers);
		}
		host = hostPortVers.substring(start, colon);
	    }
	    if (host.equals("")) {
		host = DEFAULT_HOST;
	    }
	}
    }
	 
    public Vector getAddresses() {
	return addresses;
    }

    /**
     * Returns a possibly empty but non-null string that is the "string_name"
     * portion of the URL.
     */
    public String getStringName() {
	return stringName;
    }

    public Name getCosName() throws NamingException {
	return CNCtx.parser.parse(stringName);
    }

    public IiopUrl(String url) throws MalformedURLException {
	int addrStart;
	boolean oldFormat;

	if (url.startsWith("iiopname://")) {
	    oldFormat = false;
	    addrStart = 11;
	} else if (url.startsWith("iiop://")) {
	    oldFormat = true;
	    addrStart = 7;
	} else {
	    throw new MalformedURLException("Invalid iiop/iiopname URL: " + url);
	}
	int addrEnd = url.indexOf('/', addrStart);
	if (addrEnd < 0) {
	    addrEnd = url.length();
	    stringName = "";
	} else {
	    stringName = UrlUtil.decode(url.substring(addrEnd+1));
	}
	addresses = new Vector(3);
	if (oldFormat) {
	    // Only one host:port part, not multiple
	    addresses.addElement(
		new Address(url.substring(addrStart, addrEnd), oldFormat));
	} else {
	    StringTokenizer tokens = 
		new StringTokenizer(url.substring(addrStart, addrEnd), ",");
	    while (tokens.hasMoreTokens()) {
		addresses.addElement(new Address(tokens.nextToken(), oldFormat));
	    }
	    if (addresses.size() == 0) {
		addresses.addElement(new Address("", oldFormat));
	    }
	}
    }

/*
    // for testing only
    public static void main(String[] args) {
	try {
	    IiopUrl url = new IiopUrl(args[0]);
	    Vector addrs = url.getAddresses();
	    String name = url.getStringName();

	    for (int i = 0; i < addrs.size(); i++) {
		Address addr = (Address)addrs.elementAt(i);
		System.out.println("host: " + addr.host);
		System.out.println("port: " + addr.port);
		System.out.println("version: " + addr.major + " " + addr.minor);
	    }
	    System.out.println("name: " + name);
	} catch (MalformedURLException e) {
	    e.printStackTrace();
	}
    }
*/
}
