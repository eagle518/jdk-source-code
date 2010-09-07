/*
 * @(#)MPlatformService.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.io.File;
import java.security.SecureRandom;
import java.util.HashMap;
import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedAction;

/** 
 * MPlatformService is a class that encapsulates the general service on Unix
 * for standalone application on J2SE v1.5.x or later.
 */
public class MPlatformService implements Service
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler()
    {
	return new com.sun.deploy.net.cookie.GenericCookieHandler();
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return new com.sun.deploy.net.proxy.MDefaultBrowserProxyConfig();
    }
    
    /**
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
    	return new com.sun.deploy.net.proxy.MSystemProxyHandler();
    }
    
    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
	// No system default auto proxy handler. Use dummy to parse 
	// auto proxy script manually.
	//
	return new com.sun.deploy.net.proxy.DummyAutoProxyHandler();
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
	// Check if JSS crypto is configured
        if (com.sun.deploy.security.BrowserKeystore.isJSSCryptoConfigured())
        {
           // System default signing root cert store is in Mozilla
           return new com.sun.deploy.security.MozillaSigningRootCertStore();
        }
        else
           return null;
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
	// Check if JSS crypto is configured
        if (com.sun.deploy.security.BrowserKeystore.isJSSCryptoConfigured())
        {
           // System default SSL root cert store is in Mozilla
           return new com.sun.deploy.security.MozillaSSLRootCertStore();
        }
        else
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
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	// Check if JSS crypto is configured
        if (com.sun.deploy.security.BrowserKeystore.isJSSCryptoConfigured())
        {
           KeyStore ks = (KeyStore) AccessController.doPrivileged(new PrivilegedAction()
           {
                public Object run()
                {
                   try
                   {
                        // System default client authentication key store is in Mozilla
                        return KeyStore.getInstance("MozillaMy");
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
        else
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
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // Use the default Credential Manager         
        return com.sun.deploy.security.CredentialManager.getInstance();
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
        return true;
    }

    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()
    {
	return null;
    }
}






