/*
 * @(#)MNetscape6BrowserService.java	1.46 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.util.HashMap;
import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedAction;
import com.sun.deploy.services.MPlatformService;


/** 
 * MNetscape6BrowserService is a class that encapsulates the browser service
 * in Netscape 4 on Win32.
 */
public final class MNetscape6BrowserService extends MPlatformService implements BrowserService
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler()
    {
	return new sun.plugin.net.cookie.MNetscape6CookieHandler();
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return new com.sun.deploy.net.proxy.MNetscape6ProxyConfig();
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
	return new sun.plugin.net.proxy.PluginAutoProxyHandler();
    }

    /**
     * Return browser proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler()
    {
	return new sun.plugin.net.proxy.MNetscape6BrowserProxyHandler();
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
     * Return applet context
     */
    public Object getAppletContext()
    {
	return new sun.plugin.viewer.context.MNetscape6AppletContext();
    }
    
    /**
     * Return beans context
     */
    public Object getBeansContext()
    {
	sun.plugin.viewer.context.PluginBeansContext pbc = new sun.plugin.viewer.context.PluginBeansContext();
	pbc.setPluginAppletContext(new sun.plugin.viewer.context.MNetscape6AppletContext());

	return pbc;
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
     * Return browser version.
     */
    public float getBrowserVersion()
    {
	return 6.0f;
    }

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose()
    {
	// Console should be hidden completely
	return false;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener() {
        return true;
    }

    /**
     * Browser Authenticator
     * @since 1.4.2
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
		return new MNetscape6BrowserAuthenticator();
	}

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName) {
	String name = (String)getNameMap().get(rawName);
	return (name != null)?name:rawName;
    }

    private static synchronized HashMap getNameMap() {
	if(nameMap == null) {
	    nameMap = new HashMap();

	    nameMap.put("NodeList",		    "HTMLCollection");
	    nameMap.put("HTMLOptionCollection",	    "HTMLCollection");
	    nameMap.put("HTMLInsElement",	    "HTMLModElement");
	    nameMap.put("HTMLDelElement",	    "HTMLModElement");
	    nameMap.put("HTMLSpanElement",	    "HTMLElement");
	}

	return nameMap;
    }

    private static HashMap nameMap = null;
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()
    {
	return null;
    }        
}




