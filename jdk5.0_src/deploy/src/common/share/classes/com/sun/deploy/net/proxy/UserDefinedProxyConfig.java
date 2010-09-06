/*
 * @(#)UserDefinedProxyConfig.java	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.StringTokenizer;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;



/**
 * Proxy configuration defined by users in Control Panel
 */
public final class UserDefinedProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.userdef");

	BrowserProxyInfo info = new BrowserProxyInfo();

	// Default to use browser proxy
	info.setType(ProxyType.BROWSER);

	// Refresh configuration properties
	Config.refreshIfNecessary();

        // Check if user defined proxy settings should be used
        int proxyType = Config.getIntProperty(Config.PROX_TYPE_KEY);

	// Check if direct connection is selected
	if (proxyType == ProxyType.NONE)
	{
	    info.setType(ProxyType.NONE);
	}
	else if (proxyType == ProxyType.MANUAL || proxyType == ProxyType.AUTO)
	{
	    // Set to manual by default
	    info.setType(ProxyType.MANUAL);

	    if (proxyType == ProxyType.AUTO)
	    {
		String autoConfigURL = Config.getProperty(Config.PROX_AUTOCFG_KEY);

    		Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL});

		if (autoConfigURL != null && !("".equals(autoConfigURL.trim())))
		{
		    info.setType(ProxyType.AUTO);
		    info.setAutoConfigURL(autoConfigURL);
		}
	    }

	    StringBuffer buffer = new StringBuffer();
	    boolean hasProxies = false;
	    boolean useSameProxy = Config.getBooleanProperty(Config.PROX_SAME_KEY);

	    // Retrieve HTTP settings
	    //
 	    String httpHost = Config.getProperty(Config.PROX_HTTP_HOST_KEY);
	    if (httpHost != null && httpHost.trim().length() > 0)
	    {
		hasProxies = true;

		if (useSameProxy == false)
		{
		    buffer.append("http=");
		}

		buffer.append(httpHost);

    		int httpPort = Config.getIntProperty(Config.PROX_HTTP_PORT_KEY);

		if (httpPort > 0)
		    buffer.append(":" + httpPort);
	    }

	    // Determine if use same HTTP proxy for HTTPS/FTP
	    if (useSameProxy == false)
	    {
		// Retrieve HTTPS settings
		//
 		String httpsHost = Config.getProperty(Config.PROX_HTTPS_HOST_KEY);
		if (httpsHost != null && httpsHost.trim().length() > 0)
		{
		    // Add separator
		    if (hasProxies)
			buffer.append(";");

		    hasProxies = true;

		    buffer.append("https=");
		    buffer.append(httpsHost);

    		    int httpsPort = Config.getIntProperty(Config.PROX_HTTPS_PORT_KEY);

		    if (httpsPort > 0)
			buffer.append(":" + httpsPort);
		}
	    
		// Retrieve FTP settings
		//
 		String ftpHost = Config.getProperty(Config.PROX_FTP_HOST_KEY);
		if (ftpHost != null && ftpHost.trim().length() > 0)
		{
		    // Add separator
		    if (hasProxies)
			buffer.append(";");

		    hasProxies = true;

		    buffer.append("ftp=");
		    buffer.append(ftpHost);

    		    int ftpPort = Config.getIntProperty(Config.PROX_FTP_PORT_KEY);

		    if (ftpPort > 0)
			buffer.append(":" + ftpPort);
		}
	    
		// Retrieve SOCKS settings
		//
 		String socksHost = Config.getProperty(Config.PROX_SOX_HOST_KEY);
		if (socksHost != null && socksHost.trim().length() > 0)
		{
		    // Add separator
		    if (hasProxies)
			buffer.append(";");

		    hasProxies = true;

		    buffer.append("socks=");
		    buffer.append(socksHost);

    		    int socksPort = Config.getIntProperty(Config.PROX_SOX_PORT_KEY);

		    if (socksPort > 0)
			buffer.append(":" + socksPort);
		}
	    }
	
	    String proxyList = buffer.toString();

	    Trace.msgNetPrintln("net.proxy.browser.proxyList", new Object[] {proxyList});
	    
	    if (proxyList != null && !("".equals(proxyList.trim()))) 
		ProxyUtils.parseProxyServer(proxyList, info);

	    // Retrieve proxy bypass list
	    String proxyOverride = Config.getProperty(Config.PROX_BYPASS_KEY);

	    // Check if bypass local 
	    if (Config.getBooleanProperty(Config.PROX_LOCAL_KEY))
	    {
		if (proxyOverride != null)
		    proxyOverride += ";<local>";
		else
		    proxyOverride = "<local>";
	    }

    	    Trace.msgNetPrintln("net.proxy.browser.proxyOverride", new Object[] {proxyOverride});

	    // Elements in proxy by-pass list in IE is separated by ";".
	    // Wildcard is also accepted
	    //
	    if (proxyOverride != null && !("".equals(proxyOverride.trim())))
	    {
		StringTokenizer st = new StringTokenizer(proxyOverride, ";");
		ArrayList list = new ArrayList();
		while (st.hasMoreTokens()) 
		{
		    // Convert the list to lower case
		    String item = st.nextToken().toLowerCase(java.util.Locale.ENGLISH).trim();

		    if (item != null)
		    {
			list.add(item);
		    }
		}
		
		info.setOverrides(list);
	    }
	}

	Trace.msgNetPrintln("net.proxy.loading.done");

	return info;
    }
}




