/*
 * @(#)CookieStore.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.io.PrintStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Iterator;
import java.util.TreeMap;


/**
 * CookieStore is an abstract class that represents storage for 
 * cookie. Can record, retrieve, and store cookies associated 
 * with particular URLs.
 */
abstract class CookieStore
{
    // The representation of cookies is relatively simple right now:
    // a hash table with key being the domain and the value being
    // a ArrayList of cookies for that domain.
    //
    protected transient TreeMap cookieJar = new TreeMap();

    /**
     * Record the cookie in the container of cookies.  If there
     * is already a cookie which is in the exact same domain with the
     * exact same, the cookie will be overwritten.
     */
    public void recordCookie(HttpCookie cookie) 
    {
        if (shouldRejectCookie(cookie)) 
            return;

	// Reload cookie jar if necessary
	loadCookieJar();

	String domain     = cookie.getDomain().toLowerCase();
	ArrayList cookieList = (ArrayList) cookieJar.get(domain);
    
	if (cookieList == null) 
	    cookieList = new ArrayList();
    
	if (addOrReplaceCookie(cookieList, cookie)) 
	{
	    cookieJar.put(domain, cookieList);

	    saveCookieJar();
	}
    }

    
    /**
     * Load cookie jar from storage.
     */    
    protected abstract void loadCookieJar();


    /**
     * Save cookie jar into storage.
     */    
    protected abstract void saveCookieJar();

    /** 
     * Return cookie store name.
     */
    protected abstract String getName();


    /**
     * Scans the ArrayList of cookies looking for an exact match with the
     * given cookie.  Replaces it if there is one, otherwise adds
     * one at the end.  The ArrayList is presumed to have cookies which all
     * have the same domain, so the domain of the cookie is not checked.
     * <p>
     * If this is called, it is assumed that the cookie jar is exclusively
     * held by the current thread.
     *
     * @return true if the cookie is actually set
     */
    protected boolean addOrReplaceCookie(ArrayList cookies, final HttpCookie cookie) 
    {
        int        numCookies    = cookies.size();
        String     path          = cookie.getPath();
        String     name          = cookie.getName();
        HttpCookie replaced      = null;
        int        replacedIndex = -1;

        for (int i = 0; i < numCookies; i++) 
	{
            HttpCookie existingCookie = (HttpCookie) cookies.get(i);
            String     existingPath   = existingCookie.getPath();

            if (path.equals(existingPath)) 
	    {
                String existingName = existingCookie.getName();

                if (name.equals(existingName)) 
		{
                    // need to replace this one!
                    replaced      = existingCookie;
                    replacedIndex = i;

                    break;
                }
            }
        }

        // Do the replace
        if (replaced != null) 
            cookies.set(replacedIndex, cookie);
        else 
            cookies.add(cookie);

        return true;
    }

    /**
     * Predicate function which returns true if the cookie appears to be
     * invalid somehow and should not be added to the cookie set.
     */
    protected boolean shouldRejectCookie(HttpCookie cookie) 
    {
	// Cookie must have valid domain, path and name.
	//
	if (cookie.getDomain() == null || cookie.getPath() == null || cookie.getName() == null)
	    return true;

        return false;
    }

    /**
     * Return cookies that are relevant to the URL.
     */
    public String getRelevantCookies(URL url) 
    {
    	// Reload cookie jar if necessary
	loadCookieJar();

        String host = url.getHost();

        String cookieValue = getCookiesForHost(host, url);

        // REMIND: should be careful about IP addresses here.
        int index;

        while ((index = host.indexOf('.', 1)) >= 0) 
	{
            // trim off everything up to, and including the dot.
            host = host.substring(index + 1);

	    String value = getCookiesForHost(host, url);

	    if (value != null)
	    {
		if (cookieValue == null)
		    cookieValue = value;
		else
		    cookieValue = cookieValue + "; " + value;
	    }
        }

	return cookieValue;
    }

    /**
     * Host may be a FQDN, or a partial domain name starting with a dot.
     * Adds any cookies which match the host and path to the
     * cookie set on the URL connection.
     */
    private String getCookiesForHost(String host, URL url) 
    {
        ArrayList cookieList = (ArrayList) cookieJar.get(host);

        if (cookieList == null) 
	{
            return null;
        }

        String path     = url.getFile();
        int    queryInd = path.indexOf('?');

        if (queryInd > 0) 
	{
            // strip off the part following the ?
            path = path.substring(0, queryInd);
        }

        Iterator cookies = cookieList.iterator();
        ArrayList cookiesToSend = new ArrayList(10);

        while (cookies.hasNext()) 
	{
            HttpCookie cookie     = (HttpCookie) cookies.next();
            String     cookiePath = cookie.getPath();

            if (path.startsWith(cookiePath)) 
	    {
                // The Netscape spec says that /foo should
                // match /foobar and /foo/bar.  Yuck!!!
		//
                if (!cookie.hasExpired()) 
		{
		    String protocol = url.getProtocol();
		    
		    // Secure cookie should be sent only through HTTPS
		    //
		    if (protocol.equals("https") ||
			(protocol.equals("http") && !cookie.isSecure()))
		    {
			cookiesToSend.add(cookie);
		    }
                }
            }
        }

	// Sort the cookie name/value 
	Collections.sort(cookiesToSend);

        // And send the sorted cookies...
        cookies = cookiesToSend.iterator();
        StringBuffer cookieBuffer = null;

        while (cookies.hasNext()) 
	{
            HttpCookie cookie = (HttpCookie) cookies.next();

            if (cookieBuffer == null) 
                cookieBuffer = new StringBuffer(cookie.getNameValue());
            else 
                cookieBuffer.append("; ").append(cookie.getNameValue());
        }

	if (cookieBuffer == null)
	    return null;
	else
	    return cookieBuffer.toString();
    }

    /**
     * Display content of cookie store.
     */
    public String toString()
    {
	StringBuffer sb = new StringBuffer();
	

	sb.append(getName());
	sb.append("\n[\n");

	for (Iterator iter = cookieJar.values().iterator(); iter.hasNext(); )
	{
	    List cookieList = (List) iter.next();

	    for (Iterator iter2 = cookieList.iterator(); iter2.hasNext(); )
	    {
		HttpCookie cookie = (HttpCookie) iter2.next();

		sb.append("\t");
		sb.append(cookie.toString());
		sb.append("\n");
	    }
	}

	sb.append("]");

	return sb.toString();
    }
}

