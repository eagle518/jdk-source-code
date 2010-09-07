/*
 * @(#)MacOSXSafariServiceDelegate.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import com.sun.deploy.net.proxy.*;

// FIXME: need to fill out this class -- right now its only purpose is
// to avoid NullPointerExceptions in the DynamicProxyManager with
// dragged-out applets

public class MacOSXSafariServiceDelegate extends ServiceDelegate {
    public BrowserProxyConfig getProxyConfig() {
        return new BrowserProxyConfig() {
            public BrowserProxyInfo getBrowserProxyInfo() {
                // This is sufficient to allow direct network connections
                return new BrowserProxyInfo();
            }

            public void getSystemProxy(BrowserProxyInfo bpi) {
            }
        };
    }
}
