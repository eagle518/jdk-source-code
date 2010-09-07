/*
 * @(#)BrowserProxyConfigCanonicalizer.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.net.URL;
import com.sun.deploy.util.Trace;

/**
 * Canonicalize browser proxy config.
 */
public class BrowserProxyConfigCanonicalizer implements BrowserProxyConfig 
{
    private BrowserProxyInfo bpi = new BrowserProxyInfo();

    public BrowserProxyConfigCanonicalizer(BrowserProxyConfig bpc, ProxyHandler aph)
    {
	this.bpi = bpc.getBrowserProxyInfo();
	
	//this is for bug 4980122,In windows bpi.getType() can't be SYSTEM
	//bpc here is an instance of MDefaultBrowserProxyConfig
	if(bpi.getType() == ProxyType.SYSTEM) {
        bpc.getSystemProxy(bpi);
	}
	
	// Canonicalize auto proxy setting
	canonicalizeAutoConfigProxy(bpi, aph);
    }

    /**
     * Canonicalize auto proxy settings to manually parsed proxy 
     * settings extracted from pac/js file.
     */
    private void canonicalizeAutoConfigProxy(
	BrowserProxyInfo info, ProxyHandler handler)
    {
	if (info.getType() == ProxyType.AUTO) 
	{
	    ProxyInfo[] proxyInfos = new ProxyInfo[0];

	    try
	    {
		// Retrieve settings from auto proxy script either by manually
		// parse the pac/js file, or statically using IExplorers
                // javascript engine.
		
		handler.init(info);
		proxyInfos = handler.getProxyInfo(
					new URL("http://java.sun.com"));
	    } catch (Throwable e0) {
		Trace.msgNetPrintln("net.proxy.loading.auto.error");
	    }

	    ProxyInfo pi = null;
	    if (proxyInfos.length > 0) {
		pi = proxyInfos[0];
	    }

	    // The proxy setting is only hint 
	    info.setHintOnly(true);

	    if (pi != null)
	    {
		if (pi.isSocksUsed())
		{
		    info.setSocksHost(pi.getSocksProxy());    
		    info.setSocksPort(pi.getSocksPort());    
		}
		else
		{	    
		    info.setHttpHost(pi.getProxy());
		    info.setHttpPort(pi.getPort());
		    info.setHttpsHost(pi.getProxy());
		    info.setHttpsPort(pi.getPort());
		    info.setFtpHost(pi.getProxy());
		    info.setFtpPort(pi.getPort());
		    info.setGopherHost(pi.getProxy());
		    info.setGopherPort(pi.getPort());
		}
	    }
	}
    }


    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	return bpi;
    }
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }
    
}



