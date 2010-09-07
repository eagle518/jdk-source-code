/*
 * @(#)DefaultService.java	1.37 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
     * 
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
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
    public com.sun.deploy.security.CredentialManager getCredentialManager()
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
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()
    {
	return null;
    }
}






