/*
 * @(#)Applet2BrowserService.java	1.10 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import java.lang.reflect.Field;
import java.security.SecureRandom;

import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.net.proxy.*;

import sun.plugin.services.BrowserService;

/** This implementation of the deployment Service class delegates its
    functionality to the Applet2ExecutionContext in whose context we
    are running. <P>

    Due to intertwining of various code in the plugin, it is basically
    required that this class implement BrowserService, not just
    Service. We stub out certain methods like getAppletContext() which
    are not used in the new plugin implementation. Ideally the core
    plugin code would have these dependencies removed. <P>
*/

public class Applet2BrowserService implements BrowserService {

    /** The "default" Applet2ExecutionContext is an optional argument
        which is used as a hack because some of these methods are
        occasionally called outside the scope of a running
        applet. These include isConsoleIconifiedOnClose(),
        getOfflineHandler() and the methods associated with the
        browser cert stores, which are used during resetting of the
        TrustDecider from the Java Console. The delegation chain for
        these in the context of the "real" plug-in's code in
        sun.plugin2.main.client goes down through the ServiceDelegate
        class into the old plug-in's classes like
        WIExplorerBrowserService. In the context of the standalone
        Applet2Viewer we simply return reasonable but trivial answers.
    */
    public static void install(Applet2ExecutionContext defaultContext) {
        Applet2BrowserService service = new Applet2BrowserService(defaultContext);
        try {
            Class serviceManagerClass = ServiceManager.class;
            Field f = serviceManagerClass.getDeclaredField("service");
            f.setAccessible(true);
            f.set(null, service);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler() {
        // Not used in this implementation; see Applet2ExecutionContext.get/setCookie
        return null;
    }

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig() {
        // Normally not used -- see Applet2ExecutionContext.getProxyList --
        // only used in the case where the applet has been dragged out
        // of the browser
        return getContext().getProxyConfig();
    }
    /**
     * 
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
        // Normally not used -- see Applet2ExecutionContext.getProxyList --
        // only used in the case where the applet has been dragged out
        // of the browser
        return getContext().getSystemProxyHandler();
    }
    
    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler() {
        // Normally not used -- see Applet2ExecutionContext.getProxyList --
        // only used in the case where the applet has been dragged out
        // of the browser
        return getContext().getAutoProxyHandler();
    }

    /**
     * Return browser proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() {
        // Normally not used -- see Applet2ExecutionContext.getProxyList --
        // only used in the case where the applet has been dragged out
        // of the browser
        return getContext().getBrowserProxyHandler();
    }
    
    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore() {
        return getContext().getBrowserSigningRootCertStore();
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore() {
        return getContext().getBrowserSSLRootCertStore();
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore() {
        return getContext().getBrowserTrustedCertStore();
    }

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore() {
        return getContext().getBrowserClientAuthKeyStore();
    }

    /**
     * Return browser authenticator
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() {
        return getContext().getCredentialManager();
    }
    
    /**
     * Return browser authenticator
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
        return getContext().getBrowserAuthenticator();
    }

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom() {
        // This can get called early in bootstrapping during cache upgrades
        Applet2ExecutionContext context = getContext();
        if (context != null) {
            return context.getSecureRandom();
        }
        return new SecureRandom();
    }

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer() {
        return getContext().isIExplorer();
    }

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape() {
        return getContext().isNetscape();
    }
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler() {
        // FIXME: this is called early in bootstrapping
        Applet2ExecutionContext context = getContext();
        if (context == null) {
            return null;
        }

        return context.getOfflineHandler();
    }
  
    //----------------------------------------------------------------------
    // Implementation of additional methods in
    // sun.plugin.services.BrowserService
    //

    /**
     * Return applet context
     */
    public Object getAppletContext() {
        throw new RuntimeException("Not supported in the new plugin implementation");
    }

    /**
     * Return beans context
     */
    public Object getBeansContext() {
        throw new RuntimeException("Not supported in the new plugin implementation");
    }
    
    /**
     * Return browser version.
     */
    public float getBrowserVersion() {
        return getContext().getBrowserVersion();
    }
    
    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose() {
        Applet2ExecutionContext context = getContext();
        if (context == null) {
            // FIXME: this happens during bootstrapping, so we return wrong answers
            return false;
        }
        return context.isConsoleIconifiedOnClose();
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener() {
        return getContext().installBrowserEventListener();
    }

    // With the fix to the Common DOM implementation in 6674870, this
    // method is no longer called on any instance of BrowserService.
    // When the old plug-in is removed, this method and all dependent
    // methods should be deleted.
    public String mapBrowserElement(String rawName) {
        return getContext().mapBrowserElement(rawName);
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private Applet2ExecutionContext defaultContext;

    private Applet2BrowserService(Applet2ExecutionContext defaultContext) {
        this.defaultContext = defaultContext;
    }

    private Applet2ExecutionContext getContext() {
        // This can return null during bootstrapping of the plugin.
        // Calls into this at this point:
        //   isConsoleIconifiedOnClose()
        //   getProxyConfig()
        //   getOfflineHandler()
        //   getBrowserSigningRootCertStore() (called during
        //      resetting of class loader cache from Java Console
        //      since no applet is running in the main AppContext)
        // Can see these stack traces by commenting out guard code in
        // the methods above and running the standalone Applet2Viewer.
        // For this reason we support the defaultContext in the real
        // plug-in's code, but still have fallback paths for some of
        // these calls above for the standalone case.
        Plugin2Manager manager = Plugin2Manager.getCurrentManager();
        if (manager == null)
            return defaultContext;
        return manager.getAppletExecutionContext();
    }
}
