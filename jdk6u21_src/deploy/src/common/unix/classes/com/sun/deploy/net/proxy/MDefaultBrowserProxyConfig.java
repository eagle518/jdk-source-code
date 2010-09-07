/*
 * @(#)MDefaultBrowserProxyConfig.java	1.19 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

/**
 * MDefaultBrowserProxyConfig is responsible for retrieving proxy
 * settings from default browsers on Unix.
 */
public class MDefaultBrowserProxyConfig implements BrowserProxyConfig
{
    private static String PROXY_PRO_HTTP = "http";
    private static String PROXY_PRO_FTP = "ftp";
    private static String PROXY_PRO_SOCKS = "socks";
    private static String PROXY_PRO_HTTPS = "https";
    private static String PROXY_EMPTY_URL = "";
	
    /** 
     * Return all of the available information about the Netscape internet 
     * proxy addresses.
     */
    public BrowserProxyInfo getBrowserProxyInfo() 
    {
	BrowserProxyInfo info = null;

        MFirefoxProxyConfig firefoxProxyConfig = new MFirefoxProxyConfig();
        info = firefoxProxyConfig.getBrowserProxyInfo();

        if (info.getType() == ProxyType.UNKNOWN)
        {
            MNetscape6ProxyConfig ns6ProxyConfig = new MNetscape6ProxyConfig();

            // Retrieve Netscape 6 proxy info
  	    info = ns6ProxyConfig.getBrowserProxyInfo();
        }

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
    
    public void getSystemProxy(BrowserProxyInfo info) {

	String p;
	MSystemProxyHandler mph = new MSystemProxyHandler();
	if(MSystemProxyHandler.hasSystemProxies) {
        p = mph.getSystemProxy(PROXY_PRO_HTTP,PROXY_EMPTY_URL);
        if (p!=null) {
            info.setHttpHost(getHost(p));
            info.setHttpPort(getPort(p));
        }

        p = mph.getSystemProxy(PROXY_PRO_FTP,PROXY_EMPTY_URL);
        if (p!=null) {
            info.setFtpHost(getHost(p));
            info.setFtpPort(getPort(p));
        }

        p = mph.getSystemProxy(PROXY_PRO_HTTPS,PROXY_EMPTY_URL);
        if (p!=null) {
            info.setHttpsHost(getHost(p));
            info.setHttpsPort(getPort(p));
        }

        p = mph.getSystemProxy(PROXY_PRO_SOCKS,PROXY_EMPTY_URL);
        if (p!=null) {
            info.setSocksHost(getHost(p));
            info.setSocksPort(getPort(p));
        }
	}
    }
    
    private String getHost(String address) {
    	return address.substring(0,address.indexOf(':'));
    }
    
    private int getPort(String address) {
    	String port = address.substring(address.lastIndexOf(':')+1);
    	return Integer.parseInt(port);
    }


}
