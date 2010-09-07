/*
 * @(#)DeployCookieSelector.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.deploy.net.cookie;

import java.io.IOException;
import java.net.URI;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.util.TraceLevel;

/**
 * A CookieHandler object provides a callback mechanism to hook up a
 * HTTP state management policy implementation into the HTTP protocol
 * handler. The Http state management mechanism specifies a way to
 * create a stateful session with HTTP requests and responses.
 * <p>

 * A CookieHandler can be registered with the Http protocol handler by
 * doing a
 * HttpURLConnection.setDefaultCookieHandler(CookieHandler). The
 * currently registered CookieHandler is stored as a protected field
 * in HttpURLConnection.
 *
 * For more information on HTTP state management, see <a
 * href="http://www.ietf.org/rfc/rfc2965.txt""><i>RFC&nbsp;2965: HTTP
 * State Management Mechanism</i></a>
 *
 * @since 1.5
 */
public class DeployCookieSelector extends java.net.CookieHandler 
{
    /** 
     *  Reset cookie selector.
     */
    public static synchronized void reset()
    {
	// Set cookie selector
	java.net.CookieHandler.setDefault(new DeployCookieSelector()); 
    }


    /**
     * Adds all the applicable cookies from a cookie cache for the
     * specified url in the request header.
     *
     * @param url a <code>URL</code> to send cookies to in a request
     * @param requestHeaders - an immutable map from request header
     *            field names to lists of field values representing
     *            the current request headers
     * @returns an immutable map from state management headers, Cookie
     *            or Cookie2 to a list of cookies containing state
     *            information
     * @throws	IOException if an I/O error occurs 
     * @throws  IllegalArgumentException if either argument is null
     * @see #handleResponse(URL, Map)
     */
    public synchronized Map get(URI uri, Map requestHeaders) throws IOException
    {
	// Retrieve cookie info
	//
	String cookieInfo = getCookieInfo(uri.toURL());

	HashMap map = new HashMap();

	// Add cookie header
	if (cookieInfo != null)
	{
	    ArrayList valueList = new ArrayList();
	    valueList.add(cookieInfo);

	    map.put("Cookie", valueList);
	}

	return map;
    }
 
    /**
     * Sets all the applicable cookies present in the response headers
     * into a cookie cache.
     *
     * @param url a <code>URL</code> where the cookies come from
     * @param responseHeaders an immutable map from field names to
     *            lists of field values representing the response
     *            header fields returned
     * @throws	IOException if an I/O error occurs 
     * @throws  IllegalArgumentException if either argument is null
     * @see #handleRequest(URL, Map)
     */
    public synchronized void put(URI uri, Map responseHeaders) throws IOException
    {
	// Store cookie into cookie manager from HTTP headers
	//
	for (Iterator responseIter = responseHeaders.keySet().iterator(); responseIter.hasNext(); )
	{
	    String key = (String) responseIter.next();

	    // key could be null for first line of HTTP response,
	    // and we should skip it.
	    //
	    if (key != null && key.equalsIgnoreCase("Set-Cookie"))
	    {
		List header = (List) responseHeaders.get(key);

		if (header != null)
		{
		    for (Iterator iter = header.iterator(); iter.hasNext();)
		    {
			String cookieInfo = (String) iter.next();

			// Store cookie
			if (cookieInfo != null)
			    setCookieInfo(uri.toURL(), cookieInfo);
		    }
		}
	    }
	}
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    /**
     * <p> Cookie cache. </p>
     */
    private HashMap cookieTable = new HashMap();

    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param u URL
     * @param String contains the corresponding cookie value.
     */
    protected void setCookieInfo(URL u, String value)
    {
	initializeImpl();

	if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK))
            Trace.msgNetPrintln("net.cookie.server", new Object[] {u, value});

	try
	{
            setCookieInBrowser(u, value);
	}
	catch (CookieUnavailableException e)
	{
	    System.out.println(ResourceManager.getMessage("net.cookie.ignore.setcookie"));
	}
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param u URL
     * @returns String contains the corresponding cookie value.
     */
    protected String getCookieInfo(URL u)
    {
	initializeImpl();

	String cookie = null;

	try
	{
	    // Find key to lookup cookie table
	    // To lookup the cookie, just the URL hostname and path without filename
	    String file = u.getFile();

	    int index = file.lastIndexOf('/');
	    if (index != -1) {
	        file = file.substring(0, index);
	    }

	    String key = u.getProtocol() + "://" + u.getHost() + file;

	    try
	    {
                cookie = getCookieFromBrowser(u);

		// Store cookie into cache
		//
		if (cookie != null && !cookie.equals("") && !cookie.equals("\n") && !cookie.equals("\r\n"))
		    cookieTable.put(key, cookie);
		else  {
		    cookieTable.put(key, "");
		    cookie = null;
		}
	    }
	    catch (CookieUnavailableException se)
	    {
                if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK))
		   Trace.msgNetPrintln(ResourceManager.getMessage("net.cookie.noservice"));

		// Obtain cookie from cache
		cookie = (String) cookieTable.get(key);
	    }

	    if (cookie != null && Trace.isTraceLevelEnabled(TraceLevel.NETWORK))
	    {
		Trace.msgNetPrintln("net.cookie.connect", new Object[] {u, cookie});
	    }
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}

        return cookie;
    }

    // Methods which can be easily overridden to avoid using the Service mechanism
    private CookieHandler cookieHandler = null;
    protected void initializeImpl() {
	if (cookieHandler == null)
	{
	    Service service = ServiceManager.getService();
	    cookieHandler = service.getCookieHandler();
	}
    }

    protected void setCookieInBrowser(URL u, String value) throws CookieUnavailableException {
        if (cookieHandler != null)
            cookieHandler.setCookieInfo(u, value);
    }

    protected String getCookieFromBrowser(URL u) throws CookieUnavailableException {
        if (cookieHandler != null)
            return cookieHandler.getCookieInfo(u);
        return null;
    }
}
