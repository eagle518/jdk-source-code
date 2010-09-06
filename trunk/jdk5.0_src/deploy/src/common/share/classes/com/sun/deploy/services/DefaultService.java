/*
 * @(#)DefaultService.java	1.32 04/03/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.security.SecureRandom;


/** 
 * DefaultGeneralService is a class that encapsulates the default general service.
 */
public final class DefaultService implements Service
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler()
    {
	return null;
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return null;
    }

    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
	return null;
    }

    /**
     * Return browser proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler()
    {
	return null;
    }
    
    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore()
    {
	return null;
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
	return null;
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()
    {
	return null;
    }

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	return null;
    }

    /**
     * Return browser authenticator
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator()
    {
	return null;
    }

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom()
    {
	return null;
    }

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer()
    {
        return false;
    }

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape()
    {
        return false;
    }
}






