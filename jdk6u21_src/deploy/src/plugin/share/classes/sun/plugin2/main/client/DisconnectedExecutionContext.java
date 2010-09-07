/*
 * @(#)DisconnectedExecutionContext.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.net.*;
import java.util.*;
import com.sun.deploy.config.*;
import com.sun.deploy.net.proxy.*;
import sun.plugin2.applet.context.NoopExecutionContext;

public class DisconnectedExecutionContext extends NoopExecutionContext {
    public DisconnectedExecutionContext(Map/*<String,String>*/ appletParameters,
                                        String documentBase) {
        super(appletParameters, documentBase);
    }

    public void showDocument(URL url) {
        Config.getInstance().showDocument(url.toExternalForm());
    }

    public void showDocument(URL url, String target) {
        // When we're disconnected from the browser, we don't have the
        // capability to affect the target
        showDocument(url);
    }

    public java.util.List/*<java.net.Proxy>*/ getProxyList(URL url, boolean isSocketURI) {
        // Since we can't rely on having a connection to the browser,
        // fall back to the DynamicProxyManager code, which is
        // probably a better approximation than anything else we could
        // do by hand.
        // This however pulls in a lot of code and dependencies we
        // would rather not have, so we should reconsider this in the
        // future.
        initDynamicProxyManager();
        List res = null;
        try {
            // We currently don't do a good job of hooking up the
            // support for the DynamicProxyManager on all platforms,
            // in particular through the MozillaServiceDelegate
            res = DynamicProxyManager.getProxyList(url, isSocketURI);
        } catch (Exception e) {
        }
        if (res != null) {
            return res;
        }
        // Avoid throwing NullPointerExceptions deep in the HTTP code.
        // Use a direct connection.
        return super.getProxyList(url, isSocketURI);
    }

    //----------------------------------------------------------------------
    // The following methods and their dependencies are all only in
    // support of the use of the DynamicProxyManager above, which is
    // the fallback for proxy queries in the case where we are
    // disconnected from the browser

    private static boolean initializedDynamicProxyManager;

    private static synchronized void initDynamicProxyManager() {
        if (!initializedDynamicProxyManager) {
            initializedDynamicProxyManager = true;
            DynamicProxyManager.reset();
        }
    }

    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig() {
        return ServiceDelegate.get().getProxyConfig();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler() {
        return ServiceDelegate.get().getSystemProxyHandler();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler() {
        return ServiceDelegate.get().getAutoProxyHandler();
    }

    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() {
        return ServiceDelegate.get().getBrowserProxyHandler();
    }
}
