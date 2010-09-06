/*
 * @(#)ReferralEnumeration.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import javax.naming.NamingEnumeration;

interface ReferralEnumeration extends NamingEnumeration {
    void appendUnprocessedReferrals(LdapReferralException ex);
}
