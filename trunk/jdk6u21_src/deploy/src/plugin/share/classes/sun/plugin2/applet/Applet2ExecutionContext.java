/*
 * @(#)Applet2ExecutionContext.java	1.11 10/05/20
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.security.SecureRandom;
import java.net.URL;
import java.util.List;
import java.util.Map;
import com.sun.deploy.net.cookie.*;

/** Surrounding classes must provide an implementation of this class
    to provide certain information and facilities to the
    Applet2Container and Plugin2Manager. */

public interface Applet2ExecutionContext {

    /** Retrieves the parameters of the applet as key/value pairs. This
        map includes the code base and other parameters needed for the
        applet's execution. This method may not return null.
        Implementers must assume that the returned Map may be
        mutated. */
    public Map/*<String,String>*/ getAppletParameters();

    /** Set the parameters of the applet as key/value pairs.
     */
    public void setAppletParameters(Map/*<String,String>*/ params);

    /** Returns the document base of the document which caused the
        applet to be loaded. FIXME: unclear whether this may return null
        only in error situations. */
    public String getDocumentBase(Plugin2Manager manager);

    /**
     * This method is called by the PluginProxySelector to get a list of proxy 
     * from the browser side.
     */
    public List/*<java.net.Proxy>*/ getProxyList(URL url, boolean isSocketURI);
    
    /**
     * This method is called by the PluginCookieSelector to set a
     * cookie in the browser.
     */
    public void setCookie(URL url, String value) throws CookieUnavailableException;

    /**
     * This method is called by the PluginCookieSelector to fetch a
     * cookie from the browser.
     */
    public String getCookie(URL url) throws CookieUnavailableException;

    //----------------------------------------------------------------------
    // Methods implementing support for the com.sun.deploy.services.Service
    // class
    //

    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore();

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore();

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore();

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore();

    /**
     * Return browser authenticator
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator();
    
    /**
     * Return the Credential Manager
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager();

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom();

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer();

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape();
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler();

    //----------------------------------------------------------------------
    // Methods implementing support for the com.sun.deploy.services.Service
    // class
    //

    /**
     * Return browser version.
     */
    public float getBrowserVersion();
    
    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose();

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener();

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName);

    //----------------------------------------------------------------------
    // Methods implementing support for the AppletContext interface
    //

    public void showDocument(URL url);
    public void showDocument(URL url, String target);
    public void showStatus(String status);

    public netscape.javascript.JSObject getJSObject(Plugin2Manager manager) throws netscape.javascript.JSException;

    public netscape.javascript.JSObject getOneWayJSObject(Plugin2Manager manager) throws netscape.javascript.JSException;

    //----------------------------------------------------------------------
    // These are methods for better supporting the case of dragging
    // applets out of the browser to the desktop -- added specifically
    // in support of the DisconnectedExecutionContext's use of the
    // DynamicProxyManager -- might want to reconsider this
    
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig();
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler();
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler();
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler();
}
