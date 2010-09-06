/*
 * @(#)GSSCredentialSpi.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss.spi;

import org.ietf.jgss.*;
import java.security.Provider;

/**
 * This interface is implemented by a mechanism specific credential
 * element. A GSSCredential is conceptually a container class of several
 * credential elements from different mechanisms.
 *
 * @author Mayank Upadhyay
 * @version 1.5, 12/19/03
 */
public interface GSSCredentialSpi {
    
    public Provider getProvider();

    /**
     * Called to invalidate this credential element and release
     * any system recourses and cryptographic information owned
     * by the credential.
     *
     * @exception GSSException with major codes NO_CRED and FAILURE
     */
    public void dispose() throws GSSException;
    
    /**
     * Returns the principal name for this credential. The name
     * is in mechanism specific format.
     *
     * @return GSSNameSpi representing principal name of this credential
     * @exception GSSException may be thrown
     */
    public GSSNameSpi getName() throws GSSException;
    
    /**
     * Returns the init lifetime remaining.
     *
     * @return the init lifetime remaining in seconds
     * @exception GSSException may be thrown
     */
    public int getInitLifetime() throws GSSException;
    
	
    /**
     * Returns the accept lifetime remaining.
     *
     * @return the accept lifetime remaining in seconds
     * @exception GSSException may be thrown
     */
    public int getAcceptLifetime() throws GSSException;
    
    /**
     * Determines if this credential element can be used by a context
     * initiator.
     * @return true if it can be used for initiating contexts
     */
    public boolean isInitiatorCredential() throws GSSException;
    
    /**
     * Determines if this credential element can be used by a context
     * acceptor.
     * @return true if it can be used for accepting contexts
     */
    public boolean isAcceptorCredential() throws GSSException;
    
    /**
     * Returns the oid representing the underlying credential
     * mechanism oid.
     *
     * @return the Oid for this credential mechanism
     * @exception GSSException may be thrown
     */
    public Oid getMechanism();
}
