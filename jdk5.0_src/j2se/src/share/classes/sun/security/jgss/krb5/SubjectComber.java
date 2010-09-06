/*
 * @(#)SubjectComber.java	1.8 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import javax.security.auth.kerberos.KerberosTicket;
import javax.security.auth.kerberos.KerberosKey;
import javax.security.auth.Subject;
import java.security.AccessControlContext;
import java.security.PrivilegedAction;
import java.security.AccessController;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.List;
import sun.security.action.GetBooleanAction;

/**
 * This utility looks through the current Subject and retrieves a ticket or key
 * for the desired client/server principals. 
 *
 * @author Ram Marti
 * @version 1.8, 04/01/04
 * @since 1.4.2 
 */

class SubjectComber {

    private static final boolean DEBUG = Krb5Util.DEBUG; 

    /**
     * Default constructor
     */
    private SubjectComber() {  // Cannot create one of these
    }

    static Object find(Subject subject, String serverPrincipal, 
	String clientPrincipal,	Class credClass) {

	return findAux(subject, serverPrincipal, clientPrincipal, credClass, 
	    true);
    }

    static Object findMany(Subject subject, String serverPrincipal, 
	String clientPrincipal,	Class credClass) {

	return findAux(subject, serverPrincipal, clientPrincipal, credClass,
	    false);
    }
 
    /**
     * Find the ticket or key for the specified client/server principals
     * in the subject. Returns null if the subject is null.
     *
     * @return the ticket or key
     */
    private static Object findAux(Subject subject, String serverPrincipal, 
	String clientPrincipal,	Class credClass, boolean oneOnly) {

	if (subject == null) {
	    return null;
	} else {
	    List answer = (oneOnly ? null : new ArrayList());

	    if (credClass == KerberosKey.class) {
		// We are looking for a KerberosKey credentials for the
		// serverPrincipal
		Iterator iterator =
                    subject.getPrivateCredentials(KerberosKey.class).iterator();
                while (iterator.hasNext()) {
                    KerberosKey key = (KerberosKey) iterator.next();
                    if (serverPrincipal == null ||
                        serverPrincipal.equals(key.getPrincipal().getName())) {
			 if (DEBUG) {	
			     System.out.println("Found key for "
				 + key.getPrincipal() + "(" + 
				 key.getKeyType() + ")");
			 }
			 if (oneOnly) {
			     return key;
			 } else {
			     if (serverPrincipal == null) {
				 // Record name so that keys returned will all
				 // belong to the same principal
				 serverPrincipal = 
				     key.getPrincipal().getName();
			     }
			     answer.add(key);
			 }
                    }
                }
	    } else {
	        if (credClass == KerberosTicket.class) {
		    // we are looking for a KerberosTicket credentials
		    // for client-service principal pair
		    Iterator iterator = 
			subject.getPrivateCredentials(KerberosTicket.class).iterator();
		    while (iterator.hasNext()) {
			KerberosTicket ticket = (KerberosTicket)iterator.next();
			if (DEBUG) {
			    System.out.println("Found ticket for " 
					       + ticket.getClient()
					       + " to go to "
					       + ticket.getServer()
					       + " expiring on "
					       + ticket.getEndTime());
			}
			if (!ticket.isCurrent()) {
			    // let us remove the ticket from the Subject
			    // Note that both TGT and service ticket will be
			    // removed  upon expiration
			    if (!subject.isReadOnly()) {
				iterator.remove();
				if (DEBUG) {
				    System.out.println("Removed the " +
							"expired Ticket \n" +
							ticket);
				    
				}
			    }
			} else {
			    if (ticket.getServer().getName().equals
				(serverPrincipal))  {
				
				if (clientPrincipal == null ||
				    clientPrincipal.equals(
					   ticket.getClient().getName())) {
				    if (oneOnly) {
					return ticket;
				    } else {
					// Record names so that tickets will
					// all belong to same principals
					if (clientPrincipal == null) {
					    clientPrincipal = 
						ticket.getClient().getName();
					}
					if (serverPrincipal == null) {
					    serverPrincipal = 
						ticket.getServer().getName();
					}
					answer.add(ticket);
				    }
				}
			    }
			}
		    }
		}
		
	    }
	    return answer;    
	}
    }
}
