/*
 * @(#)WPlatformService.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.security.Provider;
import java.security.SecureRandom;
import java.security.Security;
import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedAction;


/** 
 * WPlatformService is a class that encapsulates the general service on Windows
 * for standalone application on J2SE v1.5.x or later.
 */
public class WPlatformService implements Service
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
	return new com.sun.deploy.net.proxy.WDefaultBrowserProxyConfig();
    }

    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
	// System default auto proxy handler is Internet Explorer
	//
	return new com.sun.deploy.net.proxy.WIExplorerAutoProxyHandler();
    }
    /**
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
    	return null;
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
	// System default signing root cert store is in Internet Explorer
	return new com.sun.deploy.security.WIExplorerSigningRootCertStore();
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
	// System default SSL root cert store is in Internet Explorer
	return new com.sun.deploy.security.WIExplorerSSLRootCertStore();
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()
    {
	// System default trusted signing cert store is in Internet Explorer
	return new com.sun.deploy.security.WIExplorerSigningCertStore();
    }

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	KeyStore ks = (KeyStore) AccessController.doPrivileged(new PrivilegedAction()
	{
	    public Object run() 
	    {
		try
		{
		    // System default client authentication key store is in Internet Explorer
		    return KeyStore.getInstance("WIExplorerMy");
		}
		catch (KeyStoreException e)
		{
		    e.printStackTrace();
		    return null;
		}
	    }
	});    
    
	return ks;
    }
 
    /**
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // return the Windows Credential Manager   
        return com.sun.deploy.security.MSCredentialManager.getInstance();
    }  

    /**
     * Return browser authenticator.
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() 
    {
	// System default authenticator is Internet Explorer 	    
	return new com.sun.deploy.security.WIExplorerBrowserAuthenticator();
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
        return true;
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






