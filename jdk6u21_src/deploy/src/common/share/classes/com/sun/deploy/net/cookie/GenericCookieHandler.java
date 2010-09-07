/*
 * @(#)GenericCookieHandler.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.io.File;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import com.sun.deploy.config.Config;


/**
 * Generic class to hold onto HTTP cookies.  Can record, retrieve, and
 * persistently store cookies associated with particular URLs.
 *
 */
public class GenericCookieHandler implements CookieHandler 
{
    // Session cookie store (in-memory)
    private CookieStore sessionCookieStore;
    
    // Persistent cookie store
    private CookieStore persistCookieStore;

    /**
     * Create a new, empty cookie jar.
     */
    public GenericCookieHandler() 
    {
	// Create cookie store for session cookie
	sessionCookieStore = new SessionCookieStore();

	// Determine backing store for persistent cookie
	File cookieFile = new File(Config.getUserCookieFile());

	// Create cookie store for persistent cookie
	persistCookieStore = new NetscapeCookieStore(cookieFile);
    }

    /* 
     * getCookieInfo takes a particular URL and returns its cookie info.
     */
    public synchronized String getCookieInfo(URL url) throws CookieUnavailableException
    {
	try
	{
	    // Make sure URL must have a path.
	    //
	    if (url.getPath() == null || url.getPath().equals(""))
		url = new URL(url, "/");
	}
	catch(MalformedURLException e)
	{
	    throw new CookieUnavailableException(e.getMessage(), e);
	}

	return getRelevantCookies(url);
    }

    /* 
     * setCookieInfo takes a particular URL and its cookie info.
     */
    public synchronized void setCookieInfo(URL url, String value) throws CookieUnavailableException
    {
        this.recordCookie(url, value);
    }

    /**
     * Create a cookie from the cookie, and use the HttpURLConnection to
     * fill in unspecified values in the cookie with defaults.
     */
    private void recordCookie(URL url, String cookieValue) 
    {
        HttpCookie cookie = HttpCookie.create(url, cookieValue);

	// Check if cookieValue is problematic
	if (cookie == null)
	    return;

        // First, check to make sure the cookie's domain matches the
        // server's, and has the required number of '.'s
        String twodot[] = { "com", "edu", "net", "org", "gov", "mil", "int" };
        String domain = cookie.getDomain();

        if (domain == null) 
	{
            return;
        }

        domain = domain.toLowerCase();

        String host = url.getHost();

        host = host.toLowerCase();

        boolean domainOK = host.equals(domain);

        if (!domainOK && host.endsWith(domain)) 
	{
            int dotsNeeded = 2;

            for (int i = 0; i < twodot.length; i++) 
	    {
                if (domain.endsWith(twodot[i])) 
		{
                    dotsNeeded = 1;
                }
            }

            int lastChar = domain.length();

            for (; (lastChar > 0) && (dotsNeeded > 0); dotsNeeded--) 
	    {
                lastChar = domain.lastIndexOf('.', lastChar - 1);
            }

            if (lastChar > 0) 
	    {
                domainOK = true;
            }
        }

        if (domainOK) 
            recordCookie(cookie);
    }

    /**
     * Record the cookie in the in-memory container of cookies.  If there
     * is already a cookie which is in the exact same domain with the
     * exact same
     */
    private void recordCookie(HttpCookie cookie) 
    {
        // Store cookie into session cookie store
	//
	if (sessionCookieStore.shouldRejectCookie(cookie) == false)
	    sessionCookieStore.recordCookie(cookie);

        // Store cookie into persist cookie store
	//
	if (persistCookieStore.shouldRejectCookie(cookie) == false)
	    persistCookieStore.recordCookie(cookie);
    }


    /**
     * Return cookies that are relevant to the URL.
     */
    private String getRelevantCookies(URL url) 
    {
        // Retrive cookie from session cookie store
	//
        String sessionCookie = sessionCookieStore.getRelevantCookies(url);

        // Store cookie into persis cookie store
	//
        String persistCookie = persistCookieStore.getRelevantCookies(url);

	if (sessionCookie == null)
	    return persistCookie;
	else if (persistCookie == null)
	    return sessionCookie;
	else
	    return sessionCookie + "; " + persistCookie;
    }

    public static void main(String[] s)
    {
	try
	{
	    GenericCookieHandler gch = new GenericCookieHandler();
	    
	    URL url0 = new URL("http://java.sun.com/bar/index.html");
	    URL url1 = new URL("http://java.sun.com");
	    URL url2 = new URL("http://java.sun.com/xyz/bar/index.html");
	    URL url3 = new URL("https://java.sun.com");
	    URL url4 = new URL("https://java.sun.com/foo/xyz/index.html");
	    URL url5 = new URL("https://java.sun.com/foobar/xyz/index.html");
	    URL url6 = new URL("https://java.sun.com/xyz/foo/index.html");

	    URL url10 = new URL("http://xyz.sun.com/");
	    URL url11 = new URL("http://xyz.sun.com/ammo/index.html");
	
	    System.out.println("Client --> " + url0);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url0));
	    System.out.println("");

	    System.out.println("Server --> " + url0);
	    System.out.println("Set-Cookie: " + "CUSTOMER_EXPIRED=WILE_E_COYOTE_EXPIRED; path=/; expires=Wednesday, 09-Nov-99 23:12:40 GMT");
	    System.out.println("Set-Cookie: " + "CUSTOMER_NOT_EXPIRED=WILE_E_COYOTE_NOT_EXPIRED; path=/; expires=Wednesday, 09-Nov-03 23:12:40 GMT");
	    System.out.println("Set-Cookie: " + "CUSTOMER_SECURE=WILE_E_COYOTE_SECURE; path=/; secure");
	    System.out.println("");
		 
	    gch.setCookieInfo(url0, "CUSTOMER_EXPIRED=WILE_E_COYOTE_EXPIRED; path=/; expires=Wednesday, 09-Nov-99 23:12:40 GMT");	    
	    gch.setCookieInfo(url0, "CUSTOMER_NOT_EXPIRED=WILE_E_COYOTE_NOT_EXPIRED; path=/; expires=Wednesday, 09-Nov-03 23:12:40 GMT");	    
	    gch.setCookieInfo(url0, "CUSTOMER_SECURE=WILE_E_COYOTE_SECURE; path=/; secure");	    

	    System.out.println("Client --> " + url1);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url1));
	    System.out.println("");

	    System.out.println("Server --> " + url2);
	    System.out.println("Set-Cookie: " + "PART_NUMBER=ROCKET_LAUNCHER_0001; path=/");
	    System.out.println("");

	    gch.setCookieInfo(url2, "PART_NUMBER=ROCKET_LAUNCHER_0001; path=/");	    

	    System.out.println("Client --> " + url1);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url1));
	    System.out.println("");

	    System.out.println("Server --> " + url0);
	    System.out.println("Set-Cookie: " + "SHIPPING=FEDEX; path=/foo");
	    System.out.println("Set-Cookie: " + "SHIPPING_SECURE=UPS; path=/foo; secure");
	    System.out.println("");

	    gch.setCookieInfo(url0, "SHIPPING=FEDEX; path=/foo");
	    gch.setCookieInfo(url0, "SHIPPING_SECURE=UPS; path=/foo; secure");

	    System.out.println("Client --> " + url1);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url1));
	    System.out.println("");
	    System.out.println("Client --> " + url0);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url0));
	    System.out.println("");

	    System.out.println("Client --> " + url3);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url3));
	    System.out.println("");

	    System.out.println("Client --> " + url4);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url4));
	    System.out.println("");

	    System.out.println("Client --> " + url5);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url5));
	    System.out.println("");

	    System.out.println("Client --> " + url6);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url6));
	    System.out.println("");

	    System.out.println("Client --> " + url10);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url10));
	    System.out.println("");

	    System.out.println("Server --> " + url10);
	    System.out.println("Set-Cookie: " + "PART_NUMBER=ROCKET_LAUNCHER_0001; path=/");
	    System.out.println("");

	    gch.setCookieInfo(url10, "PART_NUMBER=ROCKET_LAUNCHER_0001; path=/");

	    System.out.println("Client --> " + url10);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url10));
	    System.out.println("");

	    System.out.println("Server --> " + url10);
	    System.out.println("Set-Cookie: " + "PART_NUMBER=RIDING_ROCKET_0023; path=/ammo");
	    System.out.println("");

	    gch.setCookieInfo(url10, "PART_NUMBER=RIDING_ROCKET_0023; path=/ammo");

	    System.out.println("Client --> " + url11);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url11));
	    System.out.println("");

	    System.out.println("Client --> " + url10);
	    System.out.println("Cookie	  : " + gch.getCookieInfo(url10));
	    System.out.println("");

	    System.out.println(gch.sessionCookieStore.toString());
	    System.out.println("");
	    System.out.println(gch.persistCookieStore.toString());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}
    }
}

