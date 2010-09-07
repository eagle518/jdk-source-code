/*
 * @(#)MMozillaServiceDelegate.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;
import java.security.*;
import sun.plugin2.util.NativeLibLoader;

public class MMozillaServiceDelegate extends MozillaServiceDelegate {
    private com.sun.deploy.services.MPlatformService service =
        new com.sun.deploy.services.MPlatformService();

    //----------------------------------------------------------------------
    // These methods are only present in support of disconnected
    // applets (those dragged out of the web browser)
    
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()
    {
        return service.getProxyConfig();
    }

    /**
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
        return service.getSystemProxyHandler();
    }
    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()
    {
        return service.getAutoProxyHandler();
    }

    /**
     * Return browser proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler()
    {
        return service.getBrowserProxyHandler();
    }
}
