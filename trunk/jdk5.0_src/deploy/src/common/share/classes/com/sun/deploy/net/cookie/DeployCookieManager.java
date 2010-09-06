/*
 * @(#)DeployCookieManager.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.net.URL;
import java.util.HashMap;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;


/**
 * <p> PluginCookieManager is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL.
 * </p>
 */
public class DeployCookieManager 
{
    /**
     * <p> Cookie cache. </p>
     */
    private static HashMap cookieTable = new HashMap();

    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param u URL
     * @param String contains the corresponding cookie value.
     */
    public static synchronized void setCookieInfo(URL u, String value)
    {
	initialize();	

	Trace.msgNetPrintln("net.cookie.server", new Object[] {u, value});

	try
	{
	    if (cookieHandler != null)
		cookieHandler.setCookieInfo(u, value);
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
    public static synchronized String getCookieInfo(URL u)
    {
	initialize();

	String cookie = null;

	try
	{
	    // Find key to lookup cookie table
	    String key = u.getProtocol() + u.getHost() + u.getFile();

	    // To lookup the cookie, just the URL hostname and path without filename
	    int index = key.lastIndexOf('/');

	    if (index < 0)
		return null;

	    key = key.substring(0, index);

	    try
	    {
		if (cookieHandler != null)
		    cookie = cookieHandler.getCookieInfo(u);

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
		System.out.println(ResourceManager.getMessage("net.cookie.noservice"));

		// Obtain cookie from cache
		cookie = (String) cookieTable.get(key);
	    }

	    if (cookie != null)
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

  
    // Default cookie handler
    private static CookieHandler cookieHandler = null;
          
    /**
     * Returns default cookie handler.
     */
    private static void initialize()
    {
	if (cookieHandler == null)
	{
	    Service service = ServiceManager.getService();
	    cookieHandler = service.getCookieHandler();
	}
    }
}


