/*
 * @(#)DeployProxySelector.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import java.util.ListIterator;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

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
     * This is a helper method which converts a URI object to the corresponding 
     * URL object.
     */
    protected URL getURLFromURI(URI uri, boolean isSocketURI) {
        if (uri == null) {
            return null;
        }

        int    port = uri.getPort();
        String host = uri.getHost();
        if (host == null) {
            // This is a workaround to ensure backward compatibility in two
            // cases: 1. hostnames contain non-ascii characters,
            // internationalized domain names. in which case, URI will
            // return null, see BugID 4957669; 2. Some hostnames can
            // contain '_' chars even though it's not supposed to be
            // legal, in which case URI will return null for getHost,
            // but not for getAuthority() See BugID 4913253
            String auth = uri.getAuthority();
            if (auth != null) {
                int i = auth.indexOf('@');
                if (i >= 0) {
                    auth = auth.substring(i + 1);
                }
                
                i = auth.lastIndexOf(':');
                if (i >= 0) {
                    try {
                        port = Integer.parseInt(auth.substring(i+1));
                    } catch (NumberFormatException e) {
                        port = -1;
                    }
                    auth = auth.substring(0,i);
                }
                host = auth;
            }
        }

        URL url = null;
        try {
            String scheme = uri.getScheme();
            
            // Proxy handler could only handle regular scheme in URL, e.g. http, 
            // https, ftp, etc. If the scheme is "socket" or "serversocket", we 
            // should convert the URL to http and ask the proxy handler to 
            // determine if SOCKS should be used.
            if (isSocketURI) {
		if (port == -1) {
		    url = new URL("http://" + host + "/");
		} else {
		    url = new URL("http://" + host + ":" + port + "/");
		}
	    } else {
                url = uri.toURL();
	    }
        } catch (MalformedURLException ex) {
	    ex.printStackTrace();
	} catch (IllegalArgumentException ex) {
            ex.printStackTrace();
        }

        return url;
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

        String scheme = uri.getScheme();
        boolean isSocketURI = scheme.equalsIgnoreCase("socket") ||
            scheme.equalsIgnoreCase("serversocket");

        URL url = getURLFromURI(uri, isSocketURI);
                                                
        List proxyList = null;

        try {
            // Retrieve proxy info from DynamicProxyManager
            proxyList = DynamicProxyManager.getProxyList(url, isSocketURI);
        } catch(Throwable ex) {
            ex.printStackTrace();
        }

        if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
            Trace.msgNetPrintln("net.proxy.connect", new Object[]{ uri,
                                                               proxyList.get(0) });
        }
            
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

       if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK))
           Trace.msgNetPrintln("net.proxy.connectionFailure",
                new Object[] {uri.toString() + 
                ", " + sa.toString() + ioe.toString() });
             
        try {
	    DynamicProxyManager.removeProxyFromCache( uri.toURL(), 
                                                          sa.toString() );
        } catch( Exception e ) {
                Trace.securityPrintException( e );
        }
    }
}

