/*
 * @(#)DeployProxySelector.java	1.8 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.net.Proxy;
import java.net.ProxySelector;
import java.net.SocketAddress;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URI;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import com.sun.deploy.util.Trace;

/**
 * Selects the proxy server to use, if any, when connecting to the
 * network resource referenced by a URL. A proxy selector is a
 * concrete sub-class of this class and is registered with
 * URLConnection class by invoking the {@link
 * java.net.URLConnection#setDefaultProxySelector
 * setDefaultProxySelector} method. The currently registered proxy
 * selector can be retrieved by calling {@link
 * java.net.URLConnection#getDefaultProxySelector
 * getDefaultProxySelector} method.
 * 
 * <p> When a proxy selector is registered with the URLConnection class
 *  a subclass of URLConnection class should call the 
 * {@link #select select} method for each URL request so that
 * the proxy selector can decide if a direct, or proxied connection
 * should be used. The {@link #select select} method returns
 * an iterator over a collection with the preferred connection approach.
 *
 * <p> If a connection cannot be established to a proxy (PROXY or
 * SOCKS) servers then the subclass of URLConnection should call the
 * proxy selector's {@link #connectFailed connectFailed} method to
 * notify the proxy selector that the proxy server is
 * unavailable. </p>
 *
 * @since 1.5
 */
public class DeployProxySelector extends java.net.ProxySelector 
{
    /**
     * Reset proxy selector.
     */
    public static void reset()
    {
	// Reset dynamic proxy manager
	DynamicProxyManager.reset();

	// Set proxy selector
	java.net.ProxySelector.setDefault(new DeployProxySelector()); 
    }

    /**
     * Selects all the applicable proxies based on the protocol to
     * access the resource with and a server host name to access the
     * resource at.
     *
     * @param	protocol
     *		The protocol of URL that a connection is required to
     *
     * @param	host
     *		The hostname, or literal address that a connection
     *		is required too.
     *
     * @return	A collection. Each element in the
     *		the collection is of type 
     *          {@link java.net.Proxy Proxy};
     *          when no proxy is available, the collection will
     *          contain one element of type
     *          {@link java.net.Proxy Proxy}
     *          that represents a direct connection.
     * @throws  IllegalArgumentException if either argument is null
     */
    public List select(URI uri)
    {
	if (uri == null)
	    throw new IllegalArgumentException();

	ArrayList proxyList = new ArrayList();
	Proxy proxy = Proxy.NO_PROXY;

	try
	{
            // Determine scheme of URI
            String scheme = uri.getScheme();
            final boolean isSocketURI = scheme.equalsIgnoreCase("socket") || scheme.equalsIgnoreCase("serversocket");
            URL url = null;

            // Proxy handler could only handle regular scheme in URL, e.g. http, https, ftp, etc.
            // If the scheme is "socket" or "serversocket", we should convert the URL to http
            // and ask the proxy handler to determine if SOCKS should be used.
            //
            if (isSocketURI)
                url = new URI("http", uri.getUserInfo(), uri.getHost(), uri.getPort(), uri.getPath(), uri.getQuery(), uri.getFragment()).toURL();
            else
                url = uri.toURL();

	    // Retrieve proxy info from DynamicProxyManager
	    //
	    final ProxyInfo pi = DynamicProxyManager.getProxyInfo(url);

	    if (pi.isProxyUsed())
	    {
		try 
		{
		    proxy = (Proxy) AccessController.doPrivileged(
		        new PrivilegedExceptionAction() 
		    {
			public Object run() throws IOException 
			{
			    // If URI is socket or serversocket, DIRECT is default if SOCKS is not used.
			    if (isSocketURI)
			    {
				if (pi.isSocksUsed())
			    	    return new Proxy(Proxy.Type.SOCKS, new InetSocketAddress(pi.getSocksProxy(), pi.getSocksPort()));
				else
				    return Proxy.NO_PROXY;
			    }    				
			    else
			    {
				// Use SOCKS for HTTP/HTTPS/FTP/... only if HTTP proxy is not set
				if (pi.getProxy() == null && pi.isSocksUsed())	
				    return new Proxy(Proxy.Type.SOCKS, new InetSocketAddress(pi.getSocksProxy(), pi.getSocksPort()));
				else
				    return new Proxy(Proxy.Type.HTTP, new InetSocketAddress(pi.getProxy(), pi.getPort()));
			    }		    
			}
		    });
		} 
		catch (PrivilegedActionException e) 
		{
		    //no-op
		}
	    }
	}
	catch(URISyntaxException ue)
        {
            ue.printStackTrace();
        }
	catch(MalformedURLException ex)
	{
	    ex.printStackTrace();
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}

	Trace.msgNetPrintln("net.proxy.connect", new Object[]{ uri, proxy });

	proxyList.add(proxy);

	return proxyList;
    }


   /**
     * Called to indicate that a connection could not be established
     * to a proxy/socks server. An implementation of this method can
     * temporarily remove the proxies or reorder the sequence of
     * proxies returned by select(String, String), using the address
     * and they kind of IOException given.
     *
     * @param   uri
     *          The URI that the proxy at sa failed to serve.
     * @param	sa
     *		The socket address of the proxy/SOCKS server
     *
     * @param	ioe
     *		The I/O exception thrown when the connect failed.
     * @throws IllegalArgumentException if either argument is null
     */
    public void connectFailed(URI uri, SocketAddress sa, IOException ioe)
    {
	if (uri == null || sa == null || ioe == null) {
	   throw new IllegalArgumentException("Arguments can't be null.");
	}
    }
}

