/*
 * @(#)WIExplorerAutoProxyHandler.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import com.sun.deploy.net.proxy.AbstractAutoProxyHandler;
import com.sun.deploy.net.proxy.ProxyConfigException;
import com.sun.deploy.net.proxy.ProxyInfo;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for auto proxy configuration.
 */
public final class WIExplorerAutoProxyHandler extends AbstractAutoProxyHandler
{
    /**
     * Return true if auto proxy is handled by IE.
     */
    protected boolean isIExplorer()
    {
	return true;
    }

    /**
     * Create a plain HttpURLConnection with no proxy.
     *
     * @param url URL 
     * @return HttpURLConnection object.
     */
    protected HttpURLConnection createPlainHttpURLConnection(URL url) throws IOException
    {
	// Specify no proxy for this URL, 
	//
	com.sun.deploy.net.proxy.DynamicProxyManager.setNoProxy(url);
	
	return new HttpHandler().open(url);
    }

    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
    {
	String result = null;
		    
	try
	{
	    StringBuffer buffer = new StringBuffer();
	    buffer.append(autoProxyScript);
	    buffer.append("FindProxyForURL('");
	    buffer.append(u);
	    buffer.append("','");
	    buffer.append(u.getHost());
	    buffer.append("');");

	    result = evalScript(buffer.toString());

	    return extractAutoProxySetting(result);
	}
	catch (Throwable e)
	{
	    Trace.msgNetPrintln("net.proxy.auto.result.error");

	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
    }


    /**
     * Native method to evaluate JavaScript.
     */
    private native String evalScript(String script);

    /**
     * HttpHandler to workaround protected access of openConnection()
     * method in the sun.net.www.protocol.http.Handler.
     */
    private static class HttpHandler extends sun.net.www.protocol.http.Handler
    {
	HttpURLConnection open(URL u) throws IOException 
	{
	    return (HttpURLConnection) openConnection(u);
	}
    }
}
