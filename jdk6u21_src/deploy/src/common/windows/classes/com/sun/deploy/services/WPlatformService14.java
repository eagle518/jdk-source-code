/*
 * @(#)WPlatformService14.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.security.Provider;
import java.security.SecureRandom;
import java.security.Security;
import com.sun.deploy.net.proxy.BrowserProxyConfigCanonicalizer;
import com.sun.deploy.net.proxy.WIExplorerAutoProxyHandler;
import com.sun.deploy.net.proxy.WDefaultBrowserProxyConfig;


/** 
 * WPlatformService14 is a class that encapsulates the general service on Windows
 * for standalone application on J2SE v1.4.x or earlier.
 */
public class WPlatformService14 implements Service
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler()
    {
	return new com.sun.deploy.net.cookie.IExplorerCookieHandler();
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	// Use canonicalizer to workaround auto proxy script in 1.4.
	//
	return new BrowserProxyConfigCanonicalizer(
		new WDefaultBrowserProxyConfig(), getAutoProxyHandler());
    }
    /**
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
     	// System default auto proxy handler is Internet Explorer
	// Use static auto proxy handler for standalone 1.4. 
	//
	return new WIExplorerAutoProxyHandler();
    }

    /**
     * Return browser proxy handler that handles all proxy types.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler()
    {
	// No system default browser proxy handler
	//
	return null;
    }
    
    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore()
    {
	// No system default signing root cert store for J2SE 1.4.x or earlier
	return null;
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
	// No system default SSL root cert store for J2SE 1.4.x or earlier
	return null;
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()
    {
	// No system default trusted signing cert store for J2SE 1.4.x or earlier
	return null;
    }

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	// No system default client authentication key store for J2SE 1.4.x or earlier
	return null;
    }

    /**
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // Not Supported
        return null;
    } 
    
    /**
     * Return browser authenticator.
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() 
    {
	// System default authenticator is Internet Explorer 	    
	return new com.sun.deploy.security.WIExplorerBrowserAuthenticator14();
    }

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom() 
    {
	// On Windows, MS has provided Cryto APIs to generate seed
	// for secure random generator, and it is way faster than 
	// the JDK implementation.

	// Obtain Sun service provider
	Provider provider = Security.getProvider("SUN");

	// Reset secure random support
	provider.put("SecureRandom.SHA1PRNG", "com.sun.deploy.security.WSecureRandom");	    

	return new SecureRandom();
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
	return new com.sun.deploy.net.offline.WIExplorerOfflineHandler();
    }    
}






