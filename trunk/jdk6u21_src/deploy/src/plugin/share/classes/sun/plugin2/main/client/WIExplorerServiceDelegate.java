/*
 * @(#)WIExplorerServiceDelegate.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.security.SecureRandom;
import sun.plugin.services.WIExplorerBrowserService;
import sun.plugin2.util.NativeLibLoader;

/** Internet Explorer-specific implementation of service delegate class. */

public class WIExplorerServiceDelegate extends ServiceDelegate {
    private static WIExplorerBrowserService service = new WIExplorerBrowserService();

    static {
        NativeLibLoader.load(new String[] {"deploy"});
    }

    protected WIExplorerServiceDelegate() {}

    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore() {
        return service.getBrowserSigningRootCertStore();
    }

    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore() {
        return service.getBrowserSSLRootCertStore();
    }

    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore() {
        return service.getBrowserTrustedCertStore();
    }

    public java.security.KeyStore getBrowserClientAuthKeyStore() {
        return service.getBrowserClientAuthKeyStore();
    }

    public com.sun.deploy.security.CredentialManager getCredentialManager() {
        return service.getCredentialManager();
    }

    public SecureRandom getSecureRandom() {
        return service.getSecureRandom();
    }

    public boolean isIExplorer() {
        return true;
    }

    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler() {
        return service.getOfflineHandler();
    }

    //----------------------------------------------------------------------
    // The following methods exist only in support of disconnected applets

    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig() {
        return service.getProxyConfig();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
        return service.getSystemProxyHandler();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler() {
        return service.getAutoProxyHandler();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() {
        return service.getBrowserProxyHandler();
    }
}
