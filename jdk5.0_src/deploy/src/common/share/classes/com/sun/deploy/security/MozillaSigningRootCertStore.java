/*
 * @(#)MozillaSigningRootCertStore.java	1.1 04/02/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * MozillaSigningRootCertStore is a class that represents the certificate 
 * store in Mozilla which contains root CA certificates for code 
 * signing.
 */
public final class MozillaSigningRootCertStore extends MozillaCertStore
{
    /**
     * Construct an MozillaSigningRootCertStore object.
     */
    public MozillaSigningRootCertStore()
    {
    }

    /**
     * Return name of the Internet Explorer cert store.
     */
    protected String getName()
    {
	return "ROOT";
    }
    
    /**
     *  Return true if it is a trusted signing CA cert store
     */
    protected boolean isTrustedSigningCACertStore()
    {
	return true;
    }

    /**
     *  Return true if it is a trusted SSL CA cert store
     */
    protected boolean isTrustedSSLCACertStore()
    {
	return false;
    }  
}



