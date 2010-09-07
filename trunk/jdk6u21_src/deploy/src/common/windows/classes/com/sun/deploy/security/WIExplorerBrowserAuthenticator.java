/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.net.PasswordAuthentication;
import java.net.URL;
import java.util.HashMap;
import java.util.Arrays;

public final class WIExplorerBrowserAuthenticator extends WIExplorerBrowserAuthenticator14 
{
    private static final String	PROTOCOL_HTTP = "http";
    private static final String PROTOCOL_HTTPS = "https";

    private static final int DEFAULT_HTTP_PORT = 80;
    private static final int DEFAULT_HTTPS_PORT = 443;

    private static HashMap authCache = new HashMap();

    public PasswordAuthentication getAuthentication(String protocol, String siteName, 
		int port, String scheme, String realm, URL requestingURL, boolean proxyRequest) {

        char[] credential = null;
		if(requestingURL != null) {
			credential = getCredentialFromInet(requestingURL, proxyRequest);
			if(credential != null) { 
				PasswordAuthentication pa = getPAFromCharArray(credential);
				return pa;
			}
		}

		return super.getAuthentication(protocol, siteName, port, scheme, realm, requestingURL, proxyRequest);
	}

	private char[] getCredentialFromInet(URL requestURL, boolean proxyRequest) {
        char[] credential = null;
		AuthInfoItem item = (AuthInfoItem)authCache.get(requestURL);
		if(item == null) {
			int port = requestURL.getPort();
			String protocol = requestURL.getProtocol();
			boolean isSecure = false;

			// Check to see if HTTPS is the protocol
			if ((PROTOCOL_HTTPS.equalsIgnoreCase(protocol))) {
			   isSecure = true;    
			}

			if(port == -1) {
				if(PROTOCOL_HTTP.equalsIgnoreCase(protocol)) 
					port = DEFAULT_HTTP_PORT;

				if(PROTOCOL_HTTPS.equalsIgnoreCase(protocol)) 
					port = DEFAULT_HTTPS_PORT;

			}

			item = getAuthFromInet(requestURL.getHost(), port, isSecure, 
						requestURL.getPath(), proxyRequest);
		}

		if (item == null)
		  return null;

		credential = proxyRequest?item.getProxyCredential():item.getServerCredential();
		if(item.shouldRemove()) {
			authCache.remove(item);
		} else {
			authCache.put(requestURL, item);
		}

		return credential;
	}

	private native AuthInfoItem getAuthFromInet(String siteName, int port, boolean isSecure, 
							String path, boolean isProxy);
}
