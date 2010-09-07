/*
 * @(#)ServiceDelegate.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.security.SecureRandom;
import sun.plugin2.util.BrowserType;
import sun.plugin2.util.SystemUtil;

/** This class supplies certain platform- and browser-specific
    functionality. It is unfortunate that the code on the client side
    executing the applet needs to be specialized at all, but given the
    current implementation of the BrowserKeystore
    (com/sun/deploy/security/), the HTTPS protocol handler
    (com/sun/deploy/net/protocol/https/) and related classes, it is
    infeasible to try to move the browser- or platform-specific
    portion of this code across the process boundary back into the
    browser side where it probably conceptually belongs. <P>

    This class exists principally to provide an abstraction barrier to
    the MessagePassingExecutionContext for those operations that are
    platform-specific.
*/

public class ServiceDelegate {
    private static ServiceDelegate soleInstance;

    /** This class must be initialized up front with the browser type
        in use from {@link sun.plugin2.util.BrowserType
        BrowserType}. */
    public static void initialize(int browserType) throws IllegalArgumentException {
        try {
            switch (browserType) {
                case BrowserType.DEFAULT:
                    soleInstance = new ServiceDelegate();
                    break;
                case BrowserType.INTERNET_EXPLORER:
                    // Use reflection since dependent classes aren't compiled on all platforms
                    soleInstance = (ServiceDelegate)
                        Class.forName("sun.plugin2.main.client.WIExplorerServiceDelegate").newInstance();
                    break;
                case BrowserType.MOZILLA:
                    if (SystemUtil.getOSType() == SystemUtil.WINDOWS) {
                        // This helps us pick up the MSCredentialManager
                        soleInstance = (ServiceDelegate)
                            Class.forName("sun.plugin2.main.client.WMozillaServiceDelegate").newInstance();
                    } else {
                        // This only provides proxy information for disconnected applets
                        soleInstance = (ServiceDelegate)
                            Class.forName("sun.plugin2.main.client.MMozillaServiceDelegate").newInstance();
                    }
                    break;
                case BrowserType.SAFARI_MACOSX:
                    soleInstance = (ServiceDelegate)
                        Class.forName("sun.plugin2.main.client.MacOSXSafariServiceDelegate").newInstance();
                    break;
                default:
                    // FIXME: port ServiceDelegate to your browser type
                    throw new IllegalArgumentException("Unknown browser type " + browserType);
            }
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        } catch (InstantiationException e) {
            throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }

    protected ServiceDelegate() {
    }

    public static ServiceDelegate get() {
        if (soleInstance == null) {
            throw new RuntimeException("Must call ServiceDelegate.initialize() first");
        }
        return soleInstance;
    }

    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore()     { return null; }
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()         { return null; }
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()         { return null; }
    public java.security.KeyStore            getBrowserClientAuthKeyStore()       { return null; }
    public com.sun.deploy.security.CredentialManager getCredentialManager()       { return null; }

    public SecureRandom getSecureRandom() {
        return new SecureRandom();
    }

    public boolean isIExplorer() {
        return false;
    }

    public boolean isNetscape() {
        return false;
    }

    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()          { return null; }

    // The following methods exist only in support of the
    // DynamicProxyManager and disconnected applets -- may want to
    // reconsider the DynamicProxyManager's implementation and gain
    // access simply to the auto proxy handler and the cache to avoid
    // the need to expose these
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()   { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler()  { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()    { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() { return null; }
}
