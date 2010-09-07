/*
 * @(#)DynamicProxyManager.java	1.77 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Proxy;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;
import java.util.HashMap;
import java.util.Properties;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.net.protocol.rmi.DeployRMISocketFactory;
import com.sun.deploy.util.TraceLevel;


/*
 * This class holds a reference to the default proxy handler.
 */
public class DynamicProxyManager 
{
    // Proxy info cache
    private static HashMap proxyCache = new HashMap();

    // Proxy Handler
    private static ProxyHandler handler = null;

    /**
     * Return a list of Proxy for a given URL.  This will provide a
     * list of connection options available for the given URL
     *
     * @param u URL 
     * @param isSocketURI URL 
     * @retur Proxy info for a given URL
     */
    public synchronized static List getProxyList(URL url, boolean isSocketURI)
    {
        String protocol        = url.getProtocol();
        String host            = url.getHost();
	
        StringBuffer keyBuffer = new StringBuffer();
        keyBuffer.append(protocol);
	keyBuffer.append(host);
	keyBuffer.append(url.getPort());

	String key = keyBuffer.toString();

        List pInfo = null;
	
	// Check if proxy cache should be used
	if (handler.isProxyCacheSupported()) {
	    pInfo = (List)proxyCache.get(key);
        }
    
        // If no proxy cache entry was found
	if ( pInfo == null ) {
	    try {
                // Create a Cache entry for the URL
                pInfo = new ArrayList();
                
		// No proxy if localhost
		if (!host.equals("127.0.0.1") && !host.equals("localhost"))
		{
                    // Get the list of connection methods to use when
                    // Connecting to the URL
		    ProxyInfo[] proxyArray = handler.getProxyInfo(url);
                    
                    // Add the connection methods to the cache
                    for( int i=0; i<proxyArray.length; i++ ) {
                        if (proxyArray[i].isProxyUsed()) {
                            pInfo.add(getProxy(proxyArray[i], isSocketURI));
                        } else {
                            pInfo.add(Proxy.NO_PROXY);
                        }
                    }
		} else {
                    // Local Host so just add an emtpy proxy to the cache
		    pInfo.add(Proxy.NO_PROXY);
                }
                
		// Store it in cache for further use
		proxyCache.put(key.toString(), pInfo);
	    }
	    catch (com.sun.deploy.net.proxy.ProxyUnavailableException e)
	    {
                Trace.msgNetPrintln("net.proxy.service.not_available", new Object[] {url});
                
		// Notice that we should NOT cache the result here because we should
		// determine the proxy again once the service is available
		//
		pInfo.add(Proxy.NO_PROXY);
	    }
	}

        // We must never share our cached ArrayList object with the
        // outside world due to its modification during handling of
        // connection failures
        return (List) ((ArrayList) pInfo).clone();
    }


    private static Proxy getProxy(final ProxyInfo pi, final boolean isSocketURL) {
        Proxy proxy =  null;
        try {
            proxy = (Proxy) AccessController.
                doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws IOException {
                        // If URL is socket or serversocket, DIRECT is default if SOCKS is not used.
                        if (isSocketURL) {
                            if (pi.isSocksUsed())
                                return new 
                                    Proxy(Proxy.Type.SOCKS, 
                                          new InetSocketAddress(pi.getSocksProxy(), pi.getSocksPort()));
                            else
                                return Proxy.NO_PROXY;
                        }    				
                        else {
                            // Use SOCKS for HTTP/HTTPS/FTP/... only if HTTP proxy is not set
                            if (pi.getProxy() == null && pi.isSocksUsed())	
                                return new Proxy(Proxy.Type.SOCKS, 
                                                 new InetSocketAddress(pi.getSocksProxy(), pi.getSocksPort()));
                            else
                                return new Proxy(Proxy.Type.HTTP, 
                                                 new InetSocketAddress(pi.getProxy(), pi.getPort()));
                        }
                    }
                });
        } 
        catch (PrivilegedActionException e) {
            //no-op
        }
        
        return proxy;
    }

    /**
     * Return proxy info for a given URL
     *
     * @param u URL 
     * @retur Proxy info for a given URL
     */
    public synchronized static void setNoProxy(URL url)
    {
        String key = buildProxyKey( url );
        
	// Store no-proxy in proxy cache
	//
	if (handler.isProxyCacheSupported()) {
	    List list = new ArrayList();
            list.add(Proxy.NO_PROXY);
            proxyCache.put( key, list );
        }
    }

    /**
     * Builds a key from a URL for accessing proxies in the proxy map
     * @param url the URL to build the key for
     * @return A string representing the proxy key
     */
    private static String buildProxyKey( URL url ) {
        StringBuffer keyBuffer = new StringBuffer();
	keyBuffer.append(url.getProtocol());
	keyBuffer.append(url.getHost());
	keyBuffer.append(url.getPort());

	return keyBuffer.toString();       
    } 
    
    /** 
     * Removes a connection option from the proxy cache for a given URL
     *
     * @param url - The URL to remove a connection option from
     * @param connection - The connection option to be removed
     */
    protected synchronized static void removeProxyFromCache( URL url, String connection ) {
        String key = buildProxyKey( url );
        
        // If there are any connection choices in the cache for the url
        if (proxyCache.containsKey(key)) {
            List pi = (List)proxyCache.get( key );
            ListIterator iter = pi.listIterator();
            List removedProxy = new ArrayList();
        
            // Loop through the conection options for the URL and build a list
            // of elements to be removed
            while( iter.hasNext() ) {
                Proxy proxy = (Proxy)iter.next();
                InetSocketAddress proxyAddr = (InetSocketAddress)proxy.address();
                if (proxyAddr != null && connection.contains(proxyAddr.getHostName())) {
                    removedProxy.add(proxy);
                }
            }
        
            // Remove the connection option
            iter = removedProxy.listIterator();
            while( iter.hasNext() ) {
                pi.remove( iter.next() );
            }
        
            // If the last connection option was removed for this URL remove the 
            // entry from the Map.  This will force the connection handler to
            // re-evaulate what connection options it has
            if ( pi.size() == 0 ) {
                proxyCache.remove( key );
            }
        }
    }
    
    /**
     * Reset proxy info
     */
    public synchronized static void reset()
    {
	try{
	    // Clear proxy cache
	    proxyCache.clear();

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
	   
	    switch (bpi.getType())
	    {
	    
	        case ProxyType.SYSTEM:
		    try {
	    		ProxyHandler ph = 
			    service.getSystemProxyHandler();

			if (ph == null) {
			    throw new ProxyConfigException(
			       "Unable to obtain browser proxy handler.");
			}

			handler = ph;
			handler.init(bpi);
		    } catch(ProxyConfigException e) {
	    		Trace.ignoredException(e);
		    }
		    break;
		case ProxyType.BROWSER: 
		    try
		    {
			ProxyHandler ph = service.getBrowserProxyHandler(); 

			if (ph == null)
			    throw new ProxyConfigException("Unable to obtain browser proxy handler.");
			    
			handler = ph;    
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e)
		    {
			// This should never happen
			Trace.ignoredException(e);
		    }
		    break;
		case ProxyType.NONE: 
		    try
		    {
			handler = new DirectProxyHandler();
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// This should never happen
			Trace.ignoredException(e0);
		    }
		    break;
		case ProxyType.MANUAL:
		    try
		    {
			handler = new ManualProxyHandler();
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// System.out.println(e0.toString());

			Trace.msgNetPrintln("net.proxy.loading.manual.error");

			try
			{
			    bpi.setType(ProxyType.NONE);
			    handler = new DirectProxyHandler();
			    handler.init(bpi);
			}
			catch (ProxyConfigException e1)
			{
			    // This should never happen
			    Trace.ignoredException(e1);
			}
		    }

		    break;
		case ProxyType.AUTO:
		    try
		    {
			ProxyHandler ph = service.getAutoProxyHandler(); 

			if (ph == null)
			    throw new ProxyConfigException("Unable to obtain auto proxy handler.");
			    
			handler = ph;
			handler.init(bpi);
		    }
		    catch (ProxyConfigException e0)
		    {
			// System.out.println(e0.toString());

			Trace.msgNetPrintln("net.proxy.loading.auto.error");

			try
			{
			    bpi.setType(ProxyType.MANUAL);
			    handler = new ManualProxyHandler();
			    handler.init(bpi);
			}
			catch (ProxyConfigException e1)
			{
			    // e1.printStackTrace();

			    Trace.msgNetPrintln("net.proxy.loading.manual.error");

			    try
			    {
				bpi.setType(ProxyType.NONE);
				handler = new DirectProxyHandler();
				handler.init(bpi);
			    }
			    catch (ProxyConfigException e2)
			    {
				// This should never happen
				Trace.ignoredException(e2);
			    }
			}
		    }
		    break;
    		default:
		    throw new IllegalStateException("DynamicProxyManager: Invalid Proxy Type");
	    }

	    // Set RMI socket factory for proxy.
	    try
	    {
                String p = System.getProperty("jnlp.cfg.normifactory");
                boolean setSocketFac = !("true".equals(p));
                if ((bpi.getType() != ProxyType.NONE) && setSocketFac) {
                    java.rmi.server.RMISocketFactory.setSocketFactory(
                        new DeployRMISocketFactory());
                }
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
            if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK))
                Trace.msgNetPrintln(bpi.toString());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	    com.sun.deploy.ui.UIFactory.showExceptionDialog(null,e,null,null);
	}
    }

    /**
     * setProperties set all the proxy related system properties.
     */
    private static void setProperties(BrowserProxyInfo info)
    {
	Properties props = System.getProperties();

	int proxyType = info.getType();
	StringBuffer sb = new StringBuffer();

	if (info.getHttpHost() != null)
	{
	    sb.append("http=" + info.getHttpHost());
	    if (info.getHttpPort() != -1)
		sb.append(":" + info.getHttpPort());
	}
	if (info.getHttpsHost() != null)
	{
	    sb.append(",https=" + info.getHttpsHost());
	    if (info.getHttpsPort() != -1)
		sb.append(":" + info.getHttpsPort());
	}
	if (info.getFtpHost() != null)
	{
	    sb.append(",ftp=" + info.getFtpHost());
	    if (info.getFtpPort() != -1)
		sb.append(":" + info.getFtpPort());
	}
	if (info.getGopherHost() != null)
	{
	    sb.append(",gopher=" + info.getGopherHost());
	    if (info.getGopherPort() != -1)
		sb.append(":" + info.getGopherPort());
	}
	if (info.getSocksHost() != null)
	{
	    sb.append(",socks=" + info.getSocksHost());
	    if (info.getSocksPort() != -1)
		sb.append(":" + info.getSocksPort());
	}
	
	String proxyList = sb.toString();
		
	String[] overrides = info.getOverrides();
	String proxyOverride = null;

	if (overrides != null) 
	{
	    sb = new StringBuffer();

	    boolean first = true;
	    for (int idx = 0 ; idx < overrides.length ; idx++) 
	    {
		if (idx != 0)
		    sb.append(",");    

		sb.append(overrides[idx]);
	    }

	    proxyOverride = sb.toString();
	}


	props.remove("javaplugin.proxy.config.type");
	props.remove("javaplugin.proxy.config.list");
	props.remove("javaplugin.proxy.config.bypass");

	if (proxyType == ProxyType.NONE)
	{
	    props.put("javaplugin.proxy.config.type", "direct");
	}	    
	else if (proxyType == ProxyType.MANUAL)
	{
	    props.put("javaplugin.proxy.config.type", "manual");

	    if (proxyList != null)
	        props.put("javaplugin.proxy.config.list", proxyList);

	    if (proxyOverride != null)
	        props.put("javaplugin.proxy.config.bypass", proxyOverride);
	}
	else if (proxyType == ProxyType.AUTO)
	{
	    props.put("javaplugin.proxy.config.type", "auto");
	}
	else if (proxyType == ProxyType.BROWSER)
	{
	    props.put("javaplugin.proxy.config.type", "browser");
	    
	    if (proxyList != null)
	        props.put("javaplugin.proxy.config.list", proxyList);

	    if (proxyOverride != null)
	        props.put("javaplugin.proxy.config.bypass", proxyOverride);
	}
	else
	{
	    props.put("javaplugin.proxy.config.type", "unknown");
	}
	
	System.setProperties(props);
    }
}
