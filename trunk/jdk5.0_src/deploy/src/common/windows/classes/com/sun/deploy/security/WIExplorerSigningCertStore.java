/*
 * @(#)WIExplorerSigningCertStore.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * WIExplorerSigningCertStore is a class that represents the certificate 
 * store in Internet Explorer which contains trusted MY certificates for code 
 * signing.
 */
public final class WIExplorerSigningCertStore extends WIExplorerCertStore
{
    /**
     * Construct an WIExplorerSigningCertStore object.
     */
    public WIExplorerSigningCertStore()
    {
    }

    /**
     * Return name of the Internet Explorer cert store.
     */
    protected String getName()
    {
	return "TrustedPublisher";
    }

    /**
     *  Return OID filters for extended key usage.
     */
    protected String[] getExtendedKeyUsageFilters()
    {
	return new String[] { OID_EKU_CODE_SIGNING };
    }
}



