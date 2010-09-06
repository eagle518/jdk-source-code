/*
 * @(#)Krb5MechFactory.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import javax.security.auth.kerberos.ServicePermission;
import java.security.Provider;
import sun.security.util.DerOutputStream;
import sun.security.util.ObjectIdentifier;
import java.io.IOException;

/**
 * Krb5 Mechanism plug in for JGSS
 * This is the properties object required by the JGSS framework.
 * All mechanism specific information is defined here.
 *
 * @author Mayank Upadhyay
 * @version 1.11, 12/19/03
 */

public final class Krb5MechFactory implements MechanismFactory {

    private static final boolean DEBUG = Krb5Util.DEBUG;

    static final Provider PROVIDER = 
	new sun.security.jgss.SunProvider();

    static final Oid GSS_KRB5_MECH_OID = 
	createOid("1.2.840.113554.1.2.2");

    static final Oid NT_GSS_KRB5_PRINCIPAL = 
	createOid("1.2.840.113554.1.2.2.1");
    
    private static Oid[] nameTypes = 
	new Oid[] { GSSName.NT_USER_NAME,
			GSSName.NT_HOSTBASED_SERVICE,
			GSSName.NT_EXPORT_NAME,
			NT_GSS_KRB5_PRINCIPAL};

    public GSSNameSpi getNameElement(String nameStr, Oid nameType) 
	throws GSSException {
	return Krb5NameElement.getInstance(nameStr, nameType);
    }

    public GSSNameSpi getNameElement(byte[] name, Oid nameType) 
	throws GSSException { 
	// At this point, even an exported name is stripped down to safe
	// bytes only 
	// TBD: Use encoding here?
	return Krb5NameElement.getInstance(new String(name), nameType);
    }
    
    public GSSCredentialSpi getCredentialElement(GSSNameSpi name, 
           int initLifetime, int acceptLifetime,
           int usage) throws GSSException {

	if (name != null && !(name instanceof Krb5NameElement)) {
	    name = Krb5NameElement.getInstance(name.toString(), 
				       name.getStringNameType());
	}

	Krb5CredElement credElement = null;

	if (usage == GSSCredential.INITIATE_ONLY ||
	    usage == GSSCredential.INITIATE_AND_ACCEPT) {
	    credElement =
		Krb5InitCredential.getInstance((Krb5NameElement) name,
					       initLifetime);
	    checkInitCredPermission((Krb5NameElement) credElement.getName());
	} else if (usage == GSSCredential.ACCEPT_ONLY) {
	    credElement = 
		Krb5AcceptCredential.getInstance((Krb5NameElement) name);
	    checkAcceptCredPermission((Krb5NameElement) credElement.getName(),
				      name);
	} else
	    throw new GSSException(GSSException.FAILURE, -1, 
				   "Unknown usage mode requested");

	return credElement;
    }

    private void checkInitCredPermission(Krb5NameElement name) {
	SecurityManager sm = System.getSecurityManager(); 
	if (sm != null) { 
	    String realm = (name.getKrb5PrincipalName()).getRealmAsString();
	    String tgsPrincipal = 
		new String("krbtgt/" + realm + '@' + realm);
	    ServicePermission perm =  
		new ServicePermission(tgsPrincipal, "initiate"); 
	    try {
		sm.checkPermission(perm);
	    } catch (SecurityException e) {
		if (DEBUG) {
		    System.out.println("Permission to initiate" +
                        "kerberos init credential" + e.getMessage());
		}
		throw e;
	    }
	}
    }

    private void checkAcceptCredPermission(Krb5NameElement name, 
					   GSSNameSpi originalName) {
	SecurityManager sm = System.getSecurityManager(); 
	if (sm != null) { 
	    ServicePermission perm =  
		new ServicePermission(name.toString(), "accept"); 
	    try {
		sm.checkPermission(perm);
	    } catch (SecurityException e) {
		if (originalName == null) {
		    // Don't disclose the name of the principal
		    e = new SecurityException("No permission to acquire "
				      + "Kerberos accept credential");
		    // Don't call e.initCause() with caught exception
		}
		throw e;
	    }
	}
    }

    public GSSContextSpi getMechanismContext(GSSNameSpi peer, 
			     GSSCredentialSpi myInitiatorCred, int lifetime) 
	throws GSSException {
	// TBD: Convert peer to Krb5NameElement
	// TBD: Convert myInitiatorCred to Krb5CredElement
	return new Krb5Context((Krb5NameElement)peer,
			       (Krb5CredElement)myInitiatorCred, lifetime);
    }
    
    public GSSContextSpi getMechanismContext(GSSCredentialSpi myAcceptorCred) 
	throws GSSException {
	// TBD: Convert myAcceptorCred to Krb5CredElement
	return new Krb5Context((Krb5CredElement)myAcceptorCred);
    }
    
    public GSSContextSpi getMechanismContext(byte[] exportedContext)
	throws GSSException { 
	return new Krb5Context(exportedContext);
    }


    public final Oid getMechanismOid() {
	return GSS_KRB5_MECH_OID;
    }

    public Provider getProvider() {
	return PROVIDER;
    }

    public Oid[] getNameTypes() {
	return nameTypes;
    }

    private static Oid createOid(String oidStr) {
	Oid retVal = null;
	try {
	    retVal = new Oid(oidStr);
	} catch (GSSException e) {
	    // Should not happen!
	}
	return retVal;
    }
}
