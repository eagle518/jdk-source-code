/*
 * @(#)StaticProxyManager.java	1.9 04/03/17
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import java.util.Properties;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.util.Trace;


/**
 * Proxy manager for standalone application with 
 * static proxy settings in J2SE v1.4.x or earlier.
 */
public class StaticProxyManager
{
    /**
     * Reset all the proxy related system properties.
     */
    public static void reset()
    {
	try
	{
	    // Determine if the users have set the proxy configuration 
	    // in Control Panel.
	    //
	    UserDefinedProxyConfig upc = new UserDefinedProxyConfig();

	    BrowserProxyInfo bpi = upc.getBrowserProxyInfo();

	    // Obtain service
	    Service service = com.sun.deploy.services.ServiceManager.getService();	

	    if (bpi.getType() == ProxyType.BROWSER)
	    {    
		// User have decided to use browser settings
		//

		// Obtain browser proxy config
		BrowserProxyConfig bpc = service.getProxyConfig();

		// Obtain browser proxy info
		bpi = bpc.getBrowserProxyInfo();
	    }
	    else if (bpi.getType() == ProxyType.AUTO)
	    {
		// Canonicalize auto proxy
		BrowserProxyConfigCanonicalizer bpcc = new BrowserProxyConfigCanonicalizer(upc);

		bpi = bpcc.getBrowserProxyInfo();
	    }

	    // Set RMI socket factory for proxy.
	    try
	    {
		if (bpi.getType() != ProxyType.NONE)
		    java.rmi.server.RMISocketFactory.setSocketFactory(new com.sun.deploy.net.protocol.rmi.DeployRMISocketFactory());
	    }
	    catch (Throwable e)
	    {
	    }


	    // Try to set the system properties to reflect the proxy settings
	    final BrowserProxyInfo info = bpi;

	    java.security.AccessController.doPrivileged(
		new java.security.PrivilegedAction() {
		public Object run() {
                    setProperties(info);
		    return null;
		}
	    });

	    // Display proxy configuration information 
	    // System.err.println(bpi.toString());
	    Trace.msgNetPrintln(bpi.toString());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	    com.sun.deploy.util.DialogFactory.showExceptionDialog(e);
	}
    }

    /**
     * setProperties set all the proxy related system properties.
     */
    private static void setProperties(BrowserProxyInfo info)
    {
	Properties props = System.getProperties();
	String v = System.getProperty("java.version");
	boolean oldJava = (v.startsWith("1.2") || v.startsWith("1.3"));

	switch (info.getType())
	{
	    case ProxyType.NONE:
	    {
		// Remove trust proxy
		props.remove("trustProxy");

		// Disable HTTP proxy
		if (oldJava) {
		    props.remove("proxyHost");
		    props.remove("proxyPort");
		}
		props.remove("http.proxyHost");
		props.remove("http.proxyPort");
		props.remove("http.nonProxyHosts");

		// Disable HTTPS proxy
		props.remove("https.proxyHost");
		props.remove("https.proxyPort");
		props.remove("https.nonProxyHosts");

		// Disable HTTPS proxy
		props.remove("ftp.proxyHost");
		props.remove("ftp.proxyPort");
		props.remove("ftp.nonProxyHosts");

		// Disable GOPHER proxy
		props.remove("gopherProxySet");
		props.remove("gopherProxyHost");
		props.remove("gopherProxyPort");

		// Disable SOCKS proxy
		props.remove("socksProxyHost");
		props.remove("socksProxyPort");
	    }
	    break;
	    
	    case ProxyType.MANUAL:
	    case ProxyType.AUTO:
	    {
		// Trust proxy
		props.put("trustProxy", "true");

		// HTTP proxy host
		if (info.getHttpHost() != null)
		{
		    if (oldJava) {
		        props.put("proxyHost", info.getHttpHost());
		        props.put("proxyPort", String.valueOf(info.getHttpPort()));
		    }
		    props.put("http.proxyHost", info.getHttpHost());
		    props.put("http.proxyPort", String.valueOf(info.getHttpPort()));
		    props.put("http.nonProxyHosts", info.getOverridesString());
		}

		// HTTPS proxy host
		if (info.getHttpsHost() != null)
		{
		    props.put("https.proxyHost", info.getHttpsHost());
		    props.put("https.proxyPort", String.valueOf(info.getHttpsPort()));    		  
		    props.put("https.nonProxyHosts", info.getOverridesString());	
		}

		// FTP proxy host
		if (info.getFtpHost() != null)
		{
		    props.put("ftp.proxyHost", info.getFtpHost());
		    props.put("ftp.proxyPort", String.valueOf(info.getFtpPort()));    		  
		    props.put("ftp.nonProxyHosts", info.getOverridesString());	
		}

		// GOPHER proxy host
		if (info.getGopherHost() != null)
		{
		    props.put("gopherProxySet", "true");	
		    props.put("gopherProxyHost", info.getGopherHost());
		    props.put("gopherProxyPort", String.valueOf(info.getGopherPort()));    		  
		}

		// Set SOCKS proxy host only if HTTP proxy host is not specified
		if (info.getHttpHost() == null && info.getSocksHost() != null)
		{
		    props.put("socksProxyHost", info.getSocksHost());
		    props.put("socksProxyPort", String.valueOf(info.getSocksPort()));
		}
	    }
	    break;

	    case ProxyType.BROWSER:
		throw new IllegalStateException("StaticProxyManager:  ProxyType should not be BROWSER");
	}

	// Set system properties
	System.setProperties(props);
    }
}

