/*
 * @(#)LdapNameParser.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;


import javax.naming.*;
import javax.naming.ldap.LdapName;


class LdapNameParser implements NameParser {

    public LdapNameParser() {
    }

    public Name parse(String name) throws NamingException {
	return new LdapName(name);
    }
}
