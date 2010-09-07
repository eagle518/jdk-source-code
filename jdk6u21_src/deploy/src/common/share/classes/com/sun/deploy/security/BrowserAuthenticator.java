/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.net.PasswordAuthentication;
import java.net.URL;

/**
 * BrowserAuthenticator is an interface implemented by authenticator 
 * which can retrieve/store username/password through the browser.
 */
public interface BrowserAuthenticator 
{
    public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
		String scheme, String realm, URL requestURL, boolean proxyAuthentication);
}
