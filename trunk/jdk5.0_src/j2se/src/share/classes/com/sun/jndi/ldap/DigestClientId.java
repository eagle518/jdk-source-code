/*
 * @(#)DigestClientId.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.util.Arrays; // JDK 1.2
import java.util.Hashtable;

import java.io.OutputStream;
import javax.naming.ldap.Control;

/**
 * Extends SimpleClientId to add property values specific for Digest-MD5. 
 * This includes: 
 * realm, authzid, qop, strength, maxbuffer, mutual-auth, reuse,
 * all policy-related selection properties.
 * Two DigestClientIds are identical iff they pass the SimpleClientId
 * equals() test and that all of these property values are the same.
 *
 * @author Rosanna Lee
 */
class DigestClientId extends SimpleClientId {
    private static final String[] SASL_PROPS = {
        "java.naming.security.sasl.authorizationId",
        "java.naming.security.sasl.realm",
	"javax.security.sasl.qop",
	"javax.security.sasl.strength",
	"javax.security.sasl.reuse",
	"javax.security.sasl.server.authentication",
	"javax.security.sasl.maxbuffer",
	"javax.security.sasl.policy.noplaintext",
	"javax.security.sasl.policy.noactive",
	"javax.security.sasl.policy.nodictionary",
	"javax.security.sasl.policy.noanonymous",
	"javax.security.sasl.policy.forward",
	"javax.security.sasl.policy.credentials",
    };

    final private String[] propvals;
    final private int myHash;
    private int pHash = 0;

    DigestClientId(int version, String hostname, int port, 
	String protocol, Control[] bindCtls, OutputStream trace,
	String username, Object passwd, Hashtable env) {

	super(version, hostname, port, protocol, bindCtls, trace,
	    username, passwd);

	if (env == null) {
	    propvals = null;
	} else {
	    // Could be smarter and apply default values for props
	    // but for now, we just record and check exact matches
	    propvals = new String[SASL_PROPS.length];
	    for (int i = 0; i < SASL_PROPS.length; i++) {
		propvals[i] = (String) env.get(SASL_PROPS[i]);
		if (propvals[i] != null) {
		    pHash = pHash * 31 + propvals[i].hashCode();
		}
	    }
	}
	myHash = super.hashCode() + pHash;
    }

    public boolean equals(Object obj) {
	if (!(obj instanceof DigestClientId)) {
	    return false;
	}
	DigestClientId other = (DigestClientId)obj;
	return myHash == other.myHash
	    && pHash == other.pHash
	    && super.equals(obj) 
	    && Arrays.equals(propvals, other.propvals);
    }

    public int hashCode() {
	return myHash;
    }

    public String toString() {
	if (propvals != null) {
	    StringBuffer buf = new StringBuffer();
	    for (int i = 0; i < propvals.length; i++) {
		buf.append(':');
		if (propvals[i] != null) {
		    buf.append(propvals[i]);
		}
	    }
	    return super.toString() + buf.toString();
	} else {
	    return super.toString();
	}
    }
}
