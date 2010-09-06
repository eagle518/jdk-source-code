/*
 * @(#)GSSUtil.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss;

import javax.security.auth.Subject;
import javax.security.auth.kerberos.KerberosPrincipal;
import javax.security.auth.kerberos.KerberosTicket;
import javax.security.auth.kerberos.KerberosKey;
import org.ietf.jgss.GSSName;
import org.ietf.jgss.GSSCredential;
import sun.security.jgss.GSSNameImpl;
import sun.security.jgss.GSSCredentialImpl;
import sun.security.jgss.krb5.Krb5NameElement;
import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;

/**
 * The GSSUtilImplementation that knows how to work with the internals of
 * the GSS-API. 
 */
public class GSSUtil {

    public static Subject getSubject(GSSName name,
				     GSSCredential creds) {

	HashSet principals = null;
	HashSet privCredentials = null;
	HashSet pubCredentials = new HashSet(); // empty Set

	Set gssPrincipals = null;
	Set gssCredentials = null;

	// Works only for Sun implementation right now because
	// getElements() is not defined on GSSName.
	if (name instanceof GSSNameImpl) {
	    gssPrincipals = ((GSSNameImpl) name).getElements();
	    principals = new HashSet(gssPrincipals.size());
	    populatePrincipals(principals, gssPrincipals);
	} else
	    principals = new HashSet(); // empty Set

	// Works only for Sun implementation right now because
	// getElements() is not defined on GSSCredential.
	if (creds instanceof GSSCredentialImpl) {
	    gssCredentials = ((GSSCredentialImpl) creds).getElements();
	    privCredentials = new HashSet(gssCredentials.size());
	    populateCredentials(privCredentials, gssCredentials);
	} else
	    privCredentials = new HashSet(); // empty Set

	return new Subject(false, principals, pubCredentials, privCredentials);

    }

    /**
     * Populates the set principals with elements from gssPrincipals. The
     * elements from gssPrincipals are converted to
     * javax.security.auth.kerberos.KerberosPrincipal objects and then the
     * Subject is populated with them.
     */

    private static void populatePrincipals(Set principals, 
					   Set gssPrincipals) {
	Object oldPrinc;
	KerberosPrincipal newPrinc;
	KerberosPrincipal temp;
	Iterator elements = gssPrincipals.iterator();
	while (elements.hasNext()) {
	    oldPrinc = elements.next();
	    if (oldPrinc instanceof Krb5NameElement) {
		KerberosPrincipal krbPrinc = new KerberosPrincipal
		    (((Krb5NameElement)oldPrinc).getKrb5PrincipalName().getName());
		principals.add(krbPrinc);
	    } else
		throw new UnsupportedOperationException
		    ("Unknown principal class: " + oldPrinc.getClass().getName());
	}
    }

    /**
     * Populates the set credentials with elements from gssCredentials. At
     * the same time, it converts any subclasses of KerberosTicket
     * into KerberosTicket instances and any subclasses of KerberosKey into
     * KerberosKey instances. (It is not desirable to expose the customer
     * to sun.security.jgss.krb5.Krb5InitCredential which extends
     * KerberosTicket and sun.security.jgss.krb5.Kbr5AcceptCredential which 
     * extends KerberosKey.)
     */
    private static void populateCredentials(Set credentials, 
					    Set gssCredentials) {

	Object oldCred;
	Object newCred;
	KerberosTicket tempTicket;
	KerberosKey tempKey;

	Iterator elements = gssCredentials.iterator();
	while (elements.hasNext()) {

	    oldCred = elements.next();

	    if ((oldCred instanceof KerberosTicket) &&
		!oldCred.getClass().getName().equals(
			"javax.security.auth.kerberos.KerberosTicket")) {

		tempTicket = (KerberosTicket) oldCred;
		newCred = new KerberosTicket(tempTicket.getEncoded(),
				       tempTicket.getClient(),
				       tempTicket.getServer(),
				       tempTicket.getSessionKey().getEncoded(),
				       tempTicket.getSessionKeyType(),
				       tempTicket.getFlags(),
				       tempTicket.getAuthTime(),
				       tempTicket.getStartTime(),
				       tempTicket.getEndTime(),
				       tempTicket.getRenewTill(),
				       tempTicket.getClientAddresses());
		credentials.add(newCred);

	    } else if ((oldCred instanceof KerberosKey) &&
		       !oldCred.getClass().getName().equals(
			"javax.security.auth.kerberos.KerberosKey")) {

		tempKey = (KerberosKey) oldCred;
		newCred = new KerberosKey(tempKey.getPrincipal(),
					  tempKey.getEncoded(),
					  tempKey.getKeyType(),
					  tempKey.getVersionNumber());
		credentials.add(newCred);
	    } else
		credentials.add(oldCred);

	}
    }

}
