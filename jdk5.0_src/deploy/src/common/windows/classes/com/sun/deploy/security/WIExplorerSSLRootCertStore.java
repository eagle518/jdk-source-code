/*
 * @(#)WIExplorerSSLRootCertStore.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * WIExplorerSSLRootCertStore is a class that represents the certificate 
 * store in Internet Explorer which contains SSL CA certificates for 
 * client/server authentication.
 */
public final class WIExplorerSSLRootCertStore extends WIExplorerCertStore
{
    /**
     * Construct an WIExplorerSSLRootCertStore object.
     */
    public WIExplorerSSLRootCertStore()
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
     *  Return OID filters for extended key usage.
     */
    protected String[] getExtendedKeyUsageFilters()
    {
	return new String[] { OID_EKU_SERVER_AUTH, OID_EKU_CLIENT_AUTH };
    }
}



