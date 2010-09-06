/*
 * @(#)DnsNameParser.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.dns;


import javax.naming.*;


/**
 * A name parser for DNS names.
 *
 * @author Scott Seligman
 * @version 1.5 03/12/19
 */


class DnsNameParser implements NameParser {

    public Name parse(String name) throws NamingException {
	return new DnsName(name);
    }


    // Every DnsNameParser is created equal.

    public boolean equals(Object obj) {
	return (obj instanceof DnsNameParser);
    }

    public int hashCode() {
	return DnsNameParser.class.hashCode() + 1;
    }
}
