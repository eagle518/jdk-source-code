/*
 * @(#)BrowserProxyConfigCanonicalizer.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.net.URL;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DialogFactory;

/**
 * Canonicalize browser proxy config.
 */
public class BrowserProxyConfigCanonicalizer implements BrowserProxyConfig 
{
    private BrowserProxyInfo bpi = new BrowserProxyInfo();

    public BrowserProxyConfigCanonicalizer(BrowserProxyConfig bpc)
    {
	this.bpi = bpc.getBrowserProxyInfo();

	// Canonicalize auto proxy setting
	canonicalizeAutoConfigProxy(bpi);
    }

    /**
     * Canonicalize auto proxy settings to manually parsed proxy 
     * settings extracted from pac/js file.
     */
    private void canonicalizeAutoConfigProxy(BrowserProxyInfo info)
    {
	if (info.getType() == ProxyType.AUTO)
	{
	    ProxyInfo[] proxyInfos = new ProxyInfo[0];

	    try
	    {
		// Retrieve settings from auto proxy script by manually
		// parse the pac/js file. The result is not accurate,
		// and this is why the handler is called "dummy".
		//
		ProxyHandler handler = new DummyAutoProxyHandler(); 
    
		handler.init(info);

		proxyInfos = handler.getProxyInfo(new URL("http://java.sun.com"));
	    }
	    catch (Throwable e0)
	    {
		// System.out.println(e0.toString());

		Trace.msgNetPrintln("net.proxy.loading.auto.error");
	    }

	    ProxyInfo pi = proxyInfos[0];

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
}



