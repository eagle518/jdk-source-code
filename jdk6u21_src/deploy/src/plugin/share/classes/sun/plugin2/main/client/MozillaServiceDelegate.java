/*
 * @(#)MozillaServiceDelegate.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.security.*;
import sun.plugin2.util.NativeLibLoader;

/** Mozilla browser family-specific implementation of service delegate
    class. The implementation was copied from
    sun.plugin.services.WFirefoxBrowserService and
    MNetscape6BrowserService because the functionality that was needed
    from those classes was basically platform-independent. */

public class MozillaServiceDelegate extends ServiceDelegate {
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
	// No system default trusted signing cert store is in Mozilla
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
     * Return the Crendential Manager.
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        // FIXME: may need to avoid using this code if we're running on 1.4; see
        // com.sun.deploy.services.MPlatformService14
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
     * Check if browser is NS.
     */
    public boolean isNetscape()
    {
	return true;
    }

    //----------------------------------------------------------------------
    // These methods are only present in support of disconnected
    // applets (those dragged out of the web browser)

    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()   { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler()  { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()    { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() { return null; }
}
