/*
 * @(#)AxBridgeBrowserService.java	1.22 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedAction;
import com.sun.deploy.services.WPlatformService;


/** 
 * AxBridgeBrowserService is a class that encapsulates the browser service
 * in Internet Explorer
 */
public final class AxBridgeBrowserService extends WPlatformService implements BrowserService
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
	return new com.sun.deploy.net.proxy.WIExplorerProxyConfig();
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
	return new com.sun.deploy.net.proxy.WIExplorerAutoProxyHandler();
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
     * Return applet context
     */
    public Object getAppletContext()
    {
	return new sun.plugin.viewer.context.AxBridgeAppletContext();
    }

    /**
     * Return beans context
     */
    public Object getBeansContext()
    {
	sun.plugin.viewer.context.PluginBeansContext pbc = new sun.plugin.viewer.context.PluginBeansContext();
	pbc.setPluginAppletContext(new sun.plugin.viewer.context.AxBridgeAppletContext());

	return pbc;
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
	return false;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener(){
	return false;
    }

    /**
     * Browser Authenticator 
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
	return new com.sun.deploy.security.WIExplorerBrowserAuthenticator();
    }

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName) {
	return rawName;
    }
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()
    {
	return null;
    }        
}


