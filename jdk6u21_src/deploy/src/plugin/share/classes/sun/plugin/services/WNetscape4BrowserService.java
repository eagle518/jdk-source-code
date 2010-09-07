/*
 * @(#)WNetscape4BrowserService.java	1.41 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.util.HashMap;
import com.sun.deploy.services.WPlatformService;


/** 
 * WNetscape4BrowserService is a class that encapsulates the browser service
 * in Netscape 4 on Win32.
 */
public final class WNetscape4BrowserService extends WPlatformService implements BrowserService
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler()
    {
	return new sun.plugin.net.cookie.Netscape4CookieHandler();
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return new com.sun.deploy.net.proxy.WNetscape4ProxyConfig();
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
	return new sun.plugin.net.proxy.PluginAutoProxyHandler();
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
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
	// No system default client authentication key store
	return null;
    }
    
    /**
     * Return applet context
     */
    public Object getAppletContext()
    {
	return new sun.plugin.viewer.context.NetscapeAppletContext();
    }

    /**
     * Return beans context
     */
    public Object getBeansContext()
    {
	sun.plugin.viewer.context.PluginBeansContext pbc = new sun.plugin.viewer.context.PluginBeansContext();
	pbc.setPluginAppletContext(new sun.plugin.viewer.context.NetscapeAppletContext());

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
	return 4.0f;
    }

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose()
    {
	return false;
    }

    public native boolean installBrowserEventListener();
    
    /**
     * Browser Authenticator
     * @since 1.4.2
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
	return null;
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

	    nameMap.put("self.document.forms",	    "ns4.HTMLFormCollection");
	    nameMap.put("self.document.links",	    "ns4.HTMLAnchorCollection");
	    nameMap.put("self.document.images",	    "ns4.HTMLImageCollection");
	    nameMap.put("self.document.applets",    "ns4.HTMLAppletCollection");
	    nameMap.put("self.document.anchors",    "ns4.HTMLAnchorCollection");
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

