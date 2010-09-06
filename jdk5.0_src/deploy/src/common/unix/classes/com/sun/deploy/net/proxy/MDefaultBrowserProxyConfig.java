/*
 * @(#)MDefaultBrowserProxyConfig.java	1.15 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

/**
 * MDefaultBrowserProxyConfig is responsible for retrieving proxy
 * settings from default browsers on Unix.
 */
public class MDefaultBrowserProxyConfig implements BrowserProxyConfig
{
    /** 
     * Return all of the available information about the Netscape internet 
     * proxy addresses.
     */
    public BrowserProxyInfo getBrowserProxyInfo() 
    {
	BrowserProxyInfo info = null;
	
	MNetscape6ProxyConfig ns6ProxyConfig = new MNetscape6ProxyConfig();

	// Retrieve Netscape 6 proxy info
	info = ns6ProxyConfig.getBrowserProxyInfo();

	// Fallback to Netscape 4 if proxy settings in Netscape 6 
	// cannot be retrieved.
	//
	if (info.getType() == ProxyType.UNKNOWN)
	{
	    MNetscape4ProxyConfig ns4ProxyConfig = new MNetscape4ProxyConfig();

	    info = ns4ProxyConfig.getBrowserProxyInfo();
	}

	return info;
    }
}
