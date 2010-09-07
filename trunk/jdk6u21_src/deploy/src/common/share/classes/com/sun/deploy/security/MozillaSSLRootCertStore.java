/*
 * @(#)MozillaSSLRootCertStore.java	1.1 04/02/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * MozillaSSLRootCertStore is a class that represents the certificate 
 * store in Mozilla which contains SSL CA certificates for 
 * client/server authentication.
 */
public final class MozillaSSLRootCertStore extends MozillaCertStore
{
    /**
     * Construct an MozillaSSLRootCertStore object.
     */
    public MozillaSSLRootCertStore()
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
	return false;
    }

    /**
     *  Return true if it is a trusted SSL CA cert store
     */
    protected boolean isTrustedSSLCACertStore()
    {
	return true;
    }
}



