/*
 * @(#)WIExplorerSigningRootCertStore.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * WIExplorerSigningRootCertStore is a class that represents the certificate 
 * store in Internet Explorer which contains root CA certificates for code 
 * signing.
 */
public final class WIExplorerSigningRootCertStore extends WIExplorerCertStore
{
    /**
     * Construct an WIExplorerSigningRootCertStore object.
     */
    public WIExplorerSigningRootCertStore()
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
	return new String[] { OID_EKU_CODE_SIGNING };
    }
}



