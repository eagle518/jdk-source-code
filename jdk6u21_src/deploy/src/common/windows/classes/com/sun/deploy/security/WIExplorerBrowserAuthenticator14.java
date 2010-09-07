/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.net.PasswordAuthentication;
import java.net.URL;

public class WIExplorerBrowserAuthenticator14 extends AbstractBrowserAuthenticator 
{
    public PasswordAuthentication getAuthentication(String protocol, String siteName, int port, 
			String scheme, String realm, URL requestingURL, boolean isProxy) {
        char[] credential = null;

		// IE 5.5 appends port number to siteName
		if(port != -1) {
			credential = getAuthentication(siteName + ":" + port + "/" + realm);
		}

		if(credential == null) {
			credential = getAuthentication(siteName + "/" + realm);
		}

		return getPAFromCharArray(credential); 
	}

    private native char[] getAuthentication(String key);

}
