/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.net.PasswordAuthentication;
import java.net.URL;
import com.sun.deploy.security.AbstractBrowserAuthenticator;

public final class WNetscape6BrowserAuthenticator extends AbstractBrowserAuthenticator {
    public PasswordAuthentication getAuthentication(String protocol, String siteName, int port, 
		String scheme, String realm, URL requestingURL, boolean proxy) {
        return getPAFromCharArray(getBrowserAuthentication(protocol, siteName, port, scheme, realm));
    }

    private native char[] getBrowserAuthentication(String protocol, String siteName, int port,
                                     String scheme, String realm);
}
