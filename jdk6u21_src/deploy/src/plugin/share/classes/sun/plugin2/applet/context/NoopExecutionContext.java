/*
 * @(#)NoopExecutionContext.java	1.11 10/05/20
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet.context;

import java.net.*;
import java.security.SecureRandom;
import java.util.*;
import sun.plugin2.applet.*;
import com.sun.deploy.net.cookie.*;

public class NoopExecutionContext implements Applet2ExecutionContext {
    private Map/*<String,String>*/ params;
    private String documentBase;

    public NoopExecutionContext(Map/*<String,String>*/ appletParameters,
                                String documentBase) {
        this.params = appletParameters;
        this.documentBase = documentBase;
    }

    public Map/*<String,String>*/ getAppletParameters() {
        return params;
    }

    public void setAppletParameters(Map/*<String,String>*/ p) {
        params=p;
    }

    public String getDocumentBase(Plugin2Manager manager) {
        return documentBase;
    }

    public java.util.List/*<java.net.Proxy>*/ getProxyList(URL url, boolean isSocketURI) {
        List res = new ArrayList();
        res.add(Proxy.NO_PROXY);
        return res;
    }
    public void setCookie(URL url, String value) throws CookieUnavailableException {}
    public String getCookie(URL url) throws CookieUnavailableException            { return null;  }

    //----------------------------------------------------------------------
    // Methods implementing support for the com.sun.deploy.services.Service
    // class
    //
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore()     { return null;  }
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()         { return null;  }
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()         { return null;  }
    public java.security.KeyStore getBrowserClientAuthKeyStore()                  { return null;  }
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() { return null;  }
    public com.sun.deploy.security.CredentialManager getCredentialManager()       { return null;  }
    public SecureRandom getSecureRandom() {
        return new SecureRandom();
    }
    public boolean isIExplorer()                                                  { return false; }
    public boolean isNetscape()                                                   { return false; }
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()          { return null;  }

    //----------------------------------------------------------------------
    // Methods implementing support for the
    // sun.plugin.services.BrowserService interface
    //

    /**
     * Return browser version.
     */
    public float getBrowserVersion() {
        return 1.0f;
    }
    
    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose() {
        return false;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener() {
        return false;
    }

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName) {
        return rawName;
    }

    //----------------------------------------------------------------------
    // Methods implementing support for the AppletContext interface
    //

    public void showDocument(URL url) {}
    public void showDocument(URL url, String target) {}

    public void showStatus(String status) {
        System.out.println("Applet status: " + status);
    }

    public netscape.javascript.JSObject getJSObject(Plugin2Manager manager) throws netscape.javascript.JSException { return null; }

    public netscape.javascript.JSObject getOneWayJSObject(Plugin2Manager manager) throws netscape.javascript.JSException { return null; }

    //----------------------------------------------------------------------
    // Methods in support of disconnected applets

    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()   { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler()  { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()    { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() { return null; }
}
