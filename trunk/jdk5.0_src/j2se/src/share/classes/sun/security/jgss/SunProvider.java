/*
 * @(#)SunProvider.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss;

import java.security.Provider;
import java.security.AccessController;

/**
 * Defines the Sun JGSS provider.
 * Will merger this with the Sun security provider
 * sun.security.provider.Sun when the JGSS src is merged with the JDK 
 * src.
 *
 * Mechanisms supported are:
 *
 * - Kerberos v5 as defined in RFC 1964.
 *   Oid is 1.2.840.113554.1.2.2
 *
 *   [Dummy mechanism is no longer compiled:
 * - Dummy mechanism. This is primarily useful to test a multi-mech
 *   environment.
 *   Oid is 1.3.6.1.4.1.42.2.26.1.2]
 *
 * @author Mayank Upadhyay
 * @version 1.7, 12/19/03
 */

public final class SunProvider extends Provider {

    private static final long serialVersionUID = -238911724858694198L;

    private static final String INFO = "Sun " + 
	"(Kerberos v5)";
    //	"(Kerberos v5, Dummy GSS-API Mechanism)";
    
    public SunProvider() {
	/* We are the Sun JGSS provider */
	super("SunJGSS", 1.0, INFO);

	AccessController.doPrivileged(new java.security.PrivilegedAction() {
	    public Object run() {
		put("GssApiMechanism.1.2.840.113554.1.2.2", 
		    "sun.security.jgss.krb5.Krb5MechFactory");
		/*
		  put("GssApiMechanism.1.3.6.1.4.1.42.2.26.1.2", 
		  "sun.security.jgss.dummy.DummyMechFactory");
		*/
		return null;
	    }
	});
    }
}
