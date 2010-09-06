/*
 * @(#)LdapEntry.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.util.Vector;
import javax.naming.directory.Attributes;
import javax.naming.directory.Attribute;

/**
  * A holder for an LDAP entry read from an LDAP server.
  *
  * @author Jagane Sundar
  * @author Vincent Ryan
  */
final class LdapEntry {
    String DN;
    Attributes attributes;
    Vector respCtls = null;

    LdapEntry(String DN, Attributes attrs) {
	this.DN = DN;
	this.attributes = attrs;
    }

    LdapEntry(String DN, Attributes attrs, Vector respCtls) {
	this.DN = DN;
	this.attributes = attrs;
	this.respCtls = respCtls;
    }
}

