/*
 * @(#)DummyAutoProxyHandler.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.StringTokenizer;
import com.sun.deploy.net.proxy.AbstractAutoProxyHandler;
import com.sun.deploy.net.proxy.ProxyConfigException;
import com.sun.deploy.net.proxy.ProxyInfo;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for dummy auto proxy configuration. 
 *
 * This proxy handler basically parses the pac/js file manually and extract
 * the first PROXY/SOCKS/DIRECT string, so the proxy info may not be 
 * completely accurate.
 */
public final class DummyAutoProxyHandler extends AbstractAutoProxyHandler
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
	try
	{
	    String result = null;

	    if (jsPacScript != null) 
	    {		    
		StringTokenizer st = new StringTokenizer(jsPacScript, ";", false);

		while (st.hasMoreTokens()) 
		{
		    String pattern = st.nextToken();

		    int x = pattern.indexOf("DIRECT");
		    int y = pattern.indexOf("PROXY");
		    int z = pattern.indexOf("SOCKS");

		    int i = positiveMin(x, positiveMin(y, z));
		    int j = pattern.lastIndexOf("\"");

		    // Continue the loop if no proxy info is found.
		    if (i == -1)
			continue;

		    if (j <= i)
			result = pattern.substring(i);  // Case "DIRECT;..
		    else 
			result = pattern.substring(i, j); // Case "DIRECT";

		    break;
		}
	    }

	    return extractAutoProxySetting(result);
	}
	catch (Throwable e)
	{
	    Trace.msgNetPrintln("net.proxy.auto.result.error");

	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
    }

    /**
     * Return positive minimum between two values. 
     */
    private int positiveMin(int x, int y)
    {
	if (x < 0)
	    return y;

	if (y < 0)
	    return x;

	if (x > y)
	    return y;
	else
	    return x;
    }

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


