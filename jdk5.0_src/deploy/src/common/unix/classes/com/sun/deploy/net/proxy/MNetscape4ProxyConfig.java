/*
 * @(#)MNetscape4ProxyConfig.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.File;
import com.sun.deploy.util.Trace;


/**
 * Proxy configuration for Netscape Navigator 4 on Unix
 */
public final class MNetscape4ProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ns");
	
	BrowserProxyInfo info = new BrowserProxyInfo();
	String homeDir = null; 

	// Determine home directory
        homeDir = System.getProperty("user.home");
	File prefsFile = new File(homeDir + "/.netscape/preferences.js");
	NSPreferences.parseFile(prefsFile, info, 4);

        Trace.msgNetPrintln("net.proxy.loading.done");

	return info;
    }
}
