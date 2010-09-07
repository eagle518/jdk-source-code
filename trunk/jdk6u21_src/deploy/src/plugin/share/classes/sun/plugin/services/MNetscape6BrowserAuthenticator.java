/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import java.net.PasswordAuthentication;
import com.sun.deploy.security.AbstractBrowserAuthenticator;
import java.net.URL;

public final class MNetscape6BrowserAuthenticator extends AbstractBrowserAuthenticator {
     public PasswordAuthentication getAuthentication(String protocol, String siteName, int port, 
		String scheme, String realm, URL requestingURL, boolean proxy) {
         Object[] args = new Object[5];
         args[0] = protocol;
         args[1] = siteName;
         args[2] = String.valueOf(port);
         args[3] = scheme;
         args[4] = realm;
         return getPAFromCharArray(getBrowserAuthentication(args));
     }
 
     private native char[] getBrowserAuthentication(Object[] args);
}
