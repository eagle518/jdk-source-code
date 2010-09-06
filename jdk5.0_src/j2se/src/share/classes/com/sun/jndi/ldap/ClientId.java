/*
 * @(#)ClientId.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.util.Arrays; // JDK 1.2
import java.io.OutputStream;
import javax.naming.ldap.Control;

/**
 * Represents identity information about an anonymous LDAP connection.
 * This base class contains the following information:
 * - protocol version number
 * - server's hostname (case-insensitive)
 * - server's port number
 * - prototype type (plain or ssl)
 * - controls to be sent with the LDAP bind request
 *
 * All other identity classes must be a subclass of ClientId.
 * Identity subclasses would add more distinguishing information, depending
 * on the type of authentication that the connection is to have.
 *
 * The equals() and hashCode() methods of this class and its subclasses are
 * important because they are used to determine whether two requests for
 * the same connection are identical, and thus whether the same connection
 * may be shared. This is especially important for authenticated connections
 * because a mistake would result in a serious security violation.
 *
 * @author Rosanna Lee
 */
class ClientId {
    final private int version;
    final private String hostname;
    final private int port;
    final private String protocol;
    final private Control[] bindCtls;
    final private OutputStream trace;
    final private int myHash;
    final private int ctlHash;

    ClientId(int version, String hostname, int port,
	String protocol, Control[] bindCtls, OutputStream trace) {
	this.version = version;
	this.hostname = hostname.toLowerCase();  // ignore case
	this.port = port;
	this.protocol = protocol;
	this.bindCtls = (bindCtls != null ? (Control[]) bindCtls.clone() : null);
	this.trace = trace;

	myHash = version + port
	    + (trace != null ? trace.hashCode() : 0)
	    + (this.hostname != null ? this.hostname.hashCode() : 0)
	    + (protocol != null ? protocol.hashCode() : 0)
	    + (ctlHash=hashCodeControls(bindCtls));
    }

    public boolean equals(Object obj) {
	if (!(obj instanceof ClientId)) {
	    return false;
	}

	ClientId other = (ClientId)obj;
	
	return myHash == other.myHash
	    && version == other.version
	    && port == other.port
	    && trace == other.trace
	    && (hostname == other.hostname // null OK
		|| (hostname != null && hostname.equals(other.hostname)))
	    && (protocol == other.protocol // null OK
		|| (protocol != null && protocol.equals(other.protocol)))
	    && ctlHash == other.ctlHash
	    && (equalsControls(bindCtls, other.bindCtls));
    }

    public int hashCode() {
	return myHash;
    }

    private static int hashCodeControls(Control[] c) {
	if (c == null) {
	    return 0;
	}

	int code = 0;
	for (int i = 0; i < c.length; i++) {
	    code = code * 31 + c[i].getID().hashCode();
	}
	return code;
    }

    private static boolean equalsControls(Control[] a, Control[] b) {
	if (a == b) {
	    return true;  // both null or same
	}
	if (a == null || b == null) {
	    return false; // one is non-null
	}
	if (a.length != b.length) {
	    return false;
	}

	for (int i = 0; i < a.length; i++) {
	    if (!a[i].getID().equals(b[i].getID())
		|| a[i].isCritical() != b[i].isCritical()
		|| !Arrays.equals(a[i].getEncodedValue(), 
		    b[i].getEncodedValue())) {
		return false;
	    }
	}
	return true;
    }

    private static String toStringControls(Control[] ctls) {
	if (ctls == null) {
	    return "";
	}
	StringBuffer str = new StringBuffer();
	for (int i = 0; i < ctls.length; i++) {
	    str.append(ctls[i].getID());
	    str.append(' ');
	}
	return str.toString();
    }

    public String toString() {
	return (hostname + ":" + port + ":" + 
	    (protocol != null ? protocol : "") + ":" +
	    toStringControls(bindCtls));
    }
}
