/*
 * @(#)MPlatformService14.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.io.File;
import java.security.SecureRandom;
import com.sun.deploy.net.proxy.BrowserProxyConfigCanonicalizer;
import com.sun.deploy.net.proxy.DummyAutoProxyHandler;
import com.sun.deploy.net.proxy.MDefaultBrowserProxyConfig;


/** 
 * MPlatformService14 is a class that encapsulates the general service on Unix
 * for standalone application on J2SE v1.4.x or earlier.
 */
public class MPlatformService14 implements Service
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
	// Use canonicalizer to workaround auto proxy script in 1.4.
	//
	return new BrowserProxyConfigCanonicalizer(
		new MDefaultBrowserProxyConfig(), getAutoProxyHandler());
    }

    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
	// No system default auto proxy handler for standalone 1.4. 
	// Use dummy to parse auto proxy script manually.
	//
	return new DummyAutoProxyHandler();
    }
    /**
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
    	return new com.sun.deploy.net.proxy.MSystemProxyHandler();
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
	// No system default signing root cert store
	return null;
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
	// No system default SSL root cert store
	return null;
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()
    {
	// No system default trusted signing cert store
	return null;
    }
    /**
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // not supported        
        return null;
    } 
    
    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	// No system default client authentication key store
	return null;
    }

    /**
     * Return browser authenticator.
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() 
    {
	// No system default authenticator.   
	return null;
    }

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom() 
    {
	// On Unix, some OSes (e.g. Linux) has provided /dev/urandom 
	// to generate seed for secure random generator, and it is way 
	// faster than the JDK implementation.

	try
	{
	    // Check if OS level secure random generator exists.
	    // If so, use it!!
	    //
	    File randomFile = new File("/dev/urandom");

	    if (randomFile != null && randomFile.exists())
	    {
		java.security.Security.setProperty("securerandom.source", "file:/dev/urandom");
		
	    }
	}
	catch (Throwable e)
	{
	    //    
	}

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
	return null;
    }
}






