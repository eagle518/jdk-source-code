/*
 * @(#)Krb5InitCredential.java	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import sun.security.krb5.*;
import sun.security.krb5.Config;
import javax.security.auth.kerberos.*;
import java.net.InetAddress;
import java.io.IOException;
import java.util.Date;
import javax.security.auth.Subject;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.util.Iterator;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import sun.security.jgss.LoginUtility;

/**
 * Implements the krb5 initiator credential element.
 *
 * @author Mayank Upadhyay
 * @author Ram Marti 
 * @version 1.21, 12/19/03
 * @since 1.4
 */

public class Krb5InitCredential
    extends KerberosTicket
    implements Krb5CredElement {

    private static final long serialVersionUID = 7723415700837898232L;

    private Krb5NameElement name;
    private Credentials krb5Credentials;

    private Krb5InitCredential(Krb5NameElement name, 
			       byte[] asn1Encoding,
			       KerberosPrincipal client,
			       KerberosPrincipal server,
			       byte[] sessionKey,
			       int keyType,
			       boolean[] flags,
			       Date authTime,
			       Date startTime,
			       Date endTime,
			       Date renewTill,
			       InetAddress[] clientAddresses) 
			       throws GSSException {
	super(asn1Encoding,
	      client,
	      server,
	      sessionKey,
	      keyType,
	      flags,
	      authTime,
	      startTime,
	      endTime,
	      renewTill,
	      clientAddresses);

	this.name = name;

	try {
	    // Cache this for later use by the sun.security.krb5 package.
	    krb5Credentials = new Credentials(asn1Encoding,
					      client.getName(),
					      server.getName(),
					      sessionKey,
					      keyType,
					      flags,
					      authTime,
					      startTime,
					      endTime,
					      renewTill,
					      clientAddresses);
	} catch (KrbException e) {
	    throw new GSSException(GSSException.NO_CRED, -1,
				   e.getMessage());
	} catch (IOException e) {
	    throw new GSSException(GSSException.NO_CRED, -1,
				   e.getMessage());
	}

    }

    private Krb5InitCredential(Krb5NameElement name, 
			       Credentials delegatedCred,
			       byte[] asn1Encoding,
			       KerberosPrincipal client,
			       KerberosPrincipal server,
			       byte[] sessionKey,
			       int keyType,
			       boolean[] flags,
			       Date authTime,
			       Date startTime,
			       Date endTime,
			       Date renewTill,
			       InetAddress[] clientAddresses) 
			       throws GSSException {
	super(asn1Encoding,
	      client,
	      server,
	      sessionKey,
	      keyType,
	      flags,
	      authTime,
	      startTime,
	      endTime,
	      renewTill,
	      clientAddresses);

	this.name = name;
	// A delegated cred does not have all fields set. So do not try to
	// creat new Credentials out of the delegatedCred.
	this.krb5Credentials = delegatedCred;
    }

    static Krb5InitCredential getInstance(Krb5NameElement name, 
				   int initLifetime) 
	throws GSSException {

	KerberosTicket ticket = getTgtFromSubject(name, initLifetime);
	if (ticket == null)
	    throw new GSSException(GSSException.NO_CRED, -1, 
				   "Failed to find any Kerberos Ticket");

	if (name == null) {
	    String fullName = ticket.getClient().getName();
	    name = Krb5NameElement.getInstance(fullName, 
				       Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL);
	}

	return new Krb5InitCredential(name,
				      ticket.getEncoded(),
				      ticket.getClient(),
				      ticket.getServer(),
				      ticket.getSessionKey().getEncoded(),
				      ticket.getSessionKeyType(),
				      ticket.getFlags(),
				      ticket.getAuthTime(),
				      ticket.getStartTime(),
				      ticket.getEndTime(),
				      ticket.getRenewTill(),
				      ticket.getClientAddresses());
    }

    static Krb5InitCredential getInstance(Krb5NameElement name, 
				   Credentials delegatedCred)
	throws GSSException {
	
	EncryptionKey sessionKey = delegatedCred.getSessionKey();

	/*
	 * all of the following data is optional in a KRB-CRED
	 * messages. This check for each field.
	 */

	PrincipalName cPrinc = delegatedCred.getClient();
	PrincipalName sPrinc = delegatedCred.getServer();

	KerberosPrincipal client = null;
	KerberosPrincipal server = null;

	Krb5NameElement credName = null;

	if (cPrinc != null) {
	    String fullName = cPrinc.getName();
	    credName = Krb5NameElement.getInstance(fullName, 
			       Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL);
	    client =  new KerberosPrincipal(fullName);
	}
	
	// TBD: Compare name to credName?

	if (sPrinc != null) {
	    server =
		new KerberosPrincipal(sPrinc.getName());
	}

	return new Krb5InitCredential(credName,
				      delegatedCred,
				      delegatedCred.getEncoded(),
				      client,
				      server,
				      sessionKey.getBytes(),
				      sessionKey.getEType(),
				      delegatedCred.getFlags(),
				      delegatedCred.getAuthTime(),
				      delegatedCred.getStartTime(),
				      delegatedCred.getEndTime(),
				      delegatedCred.getRenewTill(),
				      delegatedCred.getClientAddresses());
    }

    /**
     * Returns the principal name for this credential. The name
     * is in mechanism specific format.
     *
     * @return GSSNameSpi representing principal name of this credential
     * @exception GSSException may be thrown
     */
    public final GSSNameSpi getName() throws GSSException {
	return name;
    }

    /**
     * Returns the init lifetime remaining.
     *
     * @return the init lifetime remaining in seconds
     * @exception GSSException may be thrown
     */
    public int getInitLifetime() throws GSSException {
	int retVal = 0;
	retVal = (int)(getEndTime().getTime() 
		       - (new Date().getTime()));
	
	return retVal;
    }

    /**
     * Returns the accept lifetime remaining.
     *
     * @return the accept lifetime remaining in seconds
     * @exception GSSException may be thrown
     */
    public int getAcceptLifetime() throws GSSException {
	return 0;
    }

    public boolean isInitiatorCredential() throws GSSException {
	return true;
    }

    public boolean isAcceptorCredential() throws GSSException {
	return false;
    }

    /**
     * Returns the oid representing the underlying credential
     * mechanism oid.
     *
     * @return the Oid for this credential mechanism
     * @exception GSSException may be thrown
     */
    public final Oid getMechanism() {
	return Krb5MechFactory.GSS_KRB5_MECH_OID;
    }

    public final java.security.Provider getProvider() {
	return Krb5MechFactory.PROVIDER;
    }


    /**
     * Returns a sun.security.krb5.Credentials instance so that it maybe
     * used in that package for th Kerberos protocol.
     */
    Credentials getKrb5Credentials() {
	return krb5Credentials;
    }

    /*
     * TBD: Call to this.refresh() should refresh the locally cached copy
     * of krb5Credentials also.
     */

    /** 
     * Called to invalidate this credential element.
     */ 
    public void dispose() throws GSSException {
	try {
	    destroy();
	} catch (javax.security.auth.DestroyFailedException e) {
	    GSSException gssException =
		new GSSException(GSSException.FAILURE, -1,
		 "Could not destroy credentials - " + e.getMessage());
	    gssException.initCause(e);
	}
    }

    // TBD: call to this.destroy() should destroy the locally cached copy
    // of krb5Credentials and then call super.destroy().

    private static KerberosTicket getTgtFromSubject(Krb5NameElement name, 
						 int initLifetime) 
	throws GSSException {

	String realm = null;
	final String clientPrincipal;

	/*
	 * Find the TGT for the realm that the client is in. If the client
	 * name is not available, then use the default realm.
	 */
	if (name != null) {
	    clientPrincipal = (name.getKrb5PrincipalName()).getName();
	    realm = (name.getKrb5PrincipalName()).getRealmAsString();
	} else {
	    clientPrincipal = null;
	    try {
		Config config = Config.getInstance();
		realm = config.getDefaultRealm();
	    } catch (KrbException e) {
		GSSException ge = 
			new GSSException(GSSException.NO_CRED, -1,
                            "Attempt to obtain INITIATE credentials failed!" +
			    " (" + e.getMessage() + ")");
		ge.initCause(e);
		throw ge;
	    }
	}

	final String tgsPrincipal = new String("krbtgt/" + realm + "@" + realm);
	// System.out.println("Will look for ticket for: " + tgsPrincipal);

	final AccessControlContext acc = AccessController.getContext();

	try {
	    return (KerberosTicket) AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		public Object run() throws Exception {
		    return Krb5Util.getTicketFromSubject(
			LoginUtility.GSS_INITIATE_ENTRY,
			clientPrincipal, tgsPrincipal, acc);
			}});
	} catch (PrivilegedActionException e) {
	    GSSException ge = 
		new GSSException(GSSException.NO_CRED, -1,
		    "Attempt to obtain new INITIATE credentials failed!" +
		    " (" + e.getMessage() + ")");
	    ge.initCause(e.getException());
	    throw ge;
	}
    }
}
