/*
 * "@(#)Krb5Util.java	1.5	04/04/01 SMI"
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import javax.security.auth.kerberos.KerberosTicket;
import javax.security.auth.kerberos.KerberosKey;
import javax.security.auth.kerberos.KerberosPrincipal;
import javax.security.auth.Subject;
import javax.security.auth.login.LoginException;
import java.security.AccessControlContext;

import sun.security.krb5.Credentials;
import sun.security.krb5.EncryptionKey;
import sun.security.krb5.KrbException;
import sun.security.jgss.LoginUtility;
import java.io.IOException;
import java.util.List;

/**
 * Utilities for obtaining and converting Kerberos tickets.
 *
 * @version 1.5, 04/01/04
 */
public class Krb5Util {

    static final boolean DEBUG =
	((Boolean)java.security.AccessController.doPrivileged(
	    new sun.security.action.GetBooleanAction
		("sun.security.krb5.debug"))).booleanValue();

    /**
     * Default constructor
     */
    private Krb5Util() {  // Cannot create one of these
    }

    /**
     * Retrieve the service ticket for serverPrincipal from caller's Subject
     * or from Subject obtained by logging in, or if not found, via the 
     * Ticket Granting Service using the TGT obtained from the Subject.
     *
     * Caller must have permission to: 
     *    - access and update Subject's private credentials
     *    - create LoginContext
     *    - read the auth.login.defaultCallbackHandler security property
     */
    public static KerberosTicket getTicketFromSubjectAndTgs(String jaasEntry,
	String clientPrincipal, String serverPrincipal, String tgsPrincipal,
	AccessControlContext acc) 
	throws LoginException, KrbException, IOException {

	// 1. Try to find service ticket in acc subject
	Subject accSubj = Subject.getSubject(acc);
	KerberosTicket ticket = (KerberosTicket) SubjectComber.find(accSubj,
	    serverPrincipal, clientPrincipal, KerberosTicket.class);

	if (ticket != null) {
	    return ticket;  // found it
	}

	Subject loginSubj = null;
	if (!LoginUtility.useSubjectCredsOnly()) {
	    // 2. Try to get ticket from login 
	    try {
		loginSubj = LoginUtility.login(jaasEntry);
		ticket = (KerberosTicket) SubjectComber.find(loginSubj,
		    serverPrincipal, clientPrincipal, KerberosTicket.class);
		if (ticket != null) {
		    return ticket; // found it
		}
	    } catch (LoginException e) {
		// No login entry to use
		// ignore and continue
	    }
	}
	    
	// Service ticket not found in subject or login
	// Try to get TGT to acquire service ticket

	// 3. Try to get TGT from acc subject
	KerberosTicket tgt = (KerberosTicket) SubjectComber.find(accSubj,
	    tgsPrincipal, clientPrincipal, KerberosTicket.class);

	boolean fromAcc;
	if (tgt == null && loginSubj != null) {
	    // 4. Try to get TGT from login subject
	    tgt = (KerberosTicket) SubjectComber.find(loginSubj, 
		tgsPrincipal, clientPrincipal, KerberosTicket.class);
	    fromAcc = false;
	} else {
	    fromAcc = true;
	}
	    
	// 5. Try to get service ticket using TGT
	if (tgt != null) {
	    Credentials tgtCreds = ticketToCreds(tgt);
	    Credentials serviceCreds = Credentials.acquireServiceCreds(
            		serverPrincipal, tgtCreds);
	    if (serviceCreds != null) {
		ticket = credsToTicket(serviceCreds);

		// Store service ticket in acc's Subject
		if (fromAcc && accSubj != null && !accSubj.isReadOnly()) {
		    accSubj.getPrivateCredentials().add(ticket);
		}
	    }
	}
	return ticket;
    }

    /**
     * Retrieves the ticket for serverPrincipal from caller's Subject, or
     * from Subject obtained by logging in via the specified JAAS login entry.
     *
     * Caller must have permission to: 
     *    - access Subject's private credentials
     *    - create LoginContext
     *    - read the auth.login.defaultCallbackHandler security property
     */
    static KerberosTicket getTicketFromSubject(String jaasEntry,
	String clientPrincipal, String serverPrincipal,
	AccessControlContext acc) throws LoginException {

	// Try to get ticket from acc's Subject
	Subject accSubj = Subject.getSubject(acc);
	KerberosTicket ticket = (KerberosTicket)
	    SubjectComber.find(accSubj, serverPrincipal, clientPrincipal, 
		  KerberosTicket.class);

	// Try to get ticket from Subject obtained from LoginUtility
        if (ticket == null && !LoginUtility.useSubjectCredsOnly()) {
	    Subject subject = LoginUtility.login(jaasEntry);
	    ticket = (KerberosTicket) SubjectComber.find(subject, 
		serverPrincipal, clientPrincipal, KerberosTicket.class);
        }
	return ticket;
    }

    /**
     * Retrieves the caller's Subject, or Subject obtained by logging in 
     * via the specified JAAS login entry.
     *
     * Caller must have permission to: 
     *    - access the Subject
     *    - create LoginContext
     *    - read the auth.login.defaultCallbackHandler security property
     */
    public static Subject getSubject(String jaasEntry,
	AccessControlContext acc) throws LoginException {

	// Try to get the Subject from acc
	Subject subject = Subject.getSubject(acc);

	// Try to get Subject obtained from LoginUtility
        if (subject == null && !LoginUtility.useSubjectCredsOnly()) {
	    subject = LoginUtility.login(jaasEntry);
        }
	return subject;
    }

    /**
     * Retrieves the Kerberos key for serverPrincipal from caller's Subject, or
     * from Subject obtained by logging via the specified JAAS login entry.
     *
     * Caller must have permission to: 
     *    - access Subject's private credentials
     *    - create LoginContext
     *    - read the auth.login.defaultCallbackHandler security property
     */

// Replaced by getKeysFromSubject()

/*
    public static KerberosKey getKeyFromSubject(String jaasEntry,
	String serverPrincipal, AccessControlContext acc) 
		throws LoginException {

	Subject accSubj = Subject.getSubject(acc);
	KerberosKey kkey = (KerberosKey) SubjectComber.find(accSubj,
	    serverPrincipal, null, KerberosKey.class);

        if (kkey == null && !LoginUtility.useSubjectCredsOnly()) {
	    Subject subject = LoginUtility.login(jaasEntry);
	    kkey = (KerberosKey) SubjectComber.find(subject, 
		serverPrincipal, null, KerberosKey.class);
        }
	return kkey;
    }
*/

    public static KerberosKey[] getKeysFromSubject(String jaasEntry,
	String serverPrincipal, AccessControlContext acc) 
		throws LoginException {

	Subject accSubj = Subject.getSubject(acc);
 	List kkeys = (List) SubjectComber.findMany(accSubj,
	    serverPrincipal, null, KerberosKey.class);

        if (kkeys == null && !LoginUtility.useSubjectCredsOnly()) {
	    Subject subject = LoginUtility.login(jaasEntry);
	    kkeys = (List) SubjectComber.findMany(subject, 
		serverPrincipal, null, KerberosKey.class);
        }

	int len;
	if (kkeys != null && (len = kkeys.size()) > 0) {
	    KerberosKey[] keys = new KerberosKey[len];
	    kkeys.toArray(keys);
	    return keys;
	} else {
	    return null;
	}
    }

    public static KerberosTicket credsToTicket(Credentials serviceCreds) {
	EncryptionKey sessionKey =  serviceCreds.getSessionKey();
	return new KerberosTicket(
	    serviceCreds.getEncoded(),
	    new KerberosPrincipal(serviceCreds.getClient().getName()),
	    new KerberosPrincipal(serviceCreds.getServer().getName()),
	    sessionKey.getBytes(), 
	    sessionKey.getEType(), 
	    serviceCreds.getFlags(), 
	    serviceCreds.getAuthTime(), 
	    serviceCreds.getStartTime(), 
	    serviceCreds.getEndTime(), 
	    serviceCreds.getRenewTill(), 
	    serviceCreds.getClientAddresses());
    };

    public static Credentials ticketToCreds(KerberosTicket kerbTicket) 
	throws KrbException, IOException {
	return new Credentials(
	    kerbTicket.getEncoded(),
	    kerbTicket.getClient().getName(),
	    kerbTicket.getServer().getName(),
	    kerbTicket.getSessionKey().getEncoded(),
	    kerbTicket.getSessionKeyType(),
	    kerbTicket.getFlags(),
	    kerbTicket.getAuthTime(),
	    kerbTicket.getStartTime(),
	    kerbTicket.getEndTime(),
	    kerbTicket.getRenewTill(),
	    kerbTicket.getClientAddresses());
    }
}
