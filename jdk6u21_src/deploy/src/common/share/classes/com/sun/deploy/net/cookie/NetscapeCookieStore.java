/*
 * @(#)NetscapeCookieStore.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.BufferedOutputStream;
import java.io.PrintWriter;
import java.io.IOException;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Date;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.TreeMap;
import java.util.List;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;


/**
 * NetscapeCookieStore class is the persistent storage that holds onto 
 * HTTP cookies in Netscape browser cookie file format.
 *
 * The layout of Netscape's cookies.txt file is such that each line contains
 * one name-value pair. An example cookies.txt file may have an entry that
 * looks like this: 
 *
 * .netscape.com     TRUE   /  FALSE  946684799   NETSCAPE_ID  100103
 *
 * Each line represents a single piece of stored information. A tab is
 * inserted between each of the fields. 
 *
 * From left-to-right, here is what each field represents: 
 *
 * domain - The domain that created AND that can read the variable. 
 *
 * flag - A TRUE/FALSE value indicating if all machines within a given
 *        domain can access the variable. This value is set automatically by the
 *        browser, depending on the value you set for domain. Default to FALSE.
 *
 * path - The path within the domain that the variable is valid for. 
 *
 * secure - A TRUE/FALSE value indicating if a secure connection with the
 *          domain is needed to access the variable. Default to TRUE.
 *
 * expiration - The UNIX time that the variable will expire on. UNIX time is
 *              defined as the number of seconds since Jan 1, 1970 00:00:00 GMT. 
 *
 * name - The name of the cookie variable. 
 *
 * value - The value of the cookie variable. 
 */
final class NetscapeCookieStore extends CookieStore
{
    // Last access date of cookie file.
    private Date lastAccessDate = new Date(0);

    // Cookie file object.
    private File cookieFile = null;
    
    /**
     * Create a new, empty cookie jar.
     */
    NetscapeCookieStore(File cookieFile) 
    {
	this.cookieFile = cookieFile;
    }

   /**
     * Load cookie jar.
     */    
    protected void loadCookieJar()
    {
	AccessController.doPrivileged(new PrivilegedAction()
	{
    	    public Object run() 
	    {
	        loadCookieJarFromStorage();
		return null;
	    }
	});
    }

    /**
     * Load cookie jar from storage.
     */    
    private void loadCookieJarFromStorage()
    {
	// Check if file exists
	if (cookieFile.exists())
	{
	    Date fileModifiedDate = new Date(cookieFile.lastModified());

	    // Load cookie jar only if cookie file has been modified since last access
	    // 
	    if (fileModifiedDate.after(lastAccessDate))
	    {
		TreeMap tempCookieJar = new TreeMap();

		try
		{
		    // Reload cookie jar from file
		    //
		    FileInputStream fis = new FileInputStream(cookieFile);
		    InputStreamReader isr = new InputStreamReader(fis);
		    BufferedReader br = new BufferedReader(isr);
		    
		    String line = null;

		    while ((line = br.readLine()) != null)
		    {
			// Skip comments
			if (line.startsWith("#"))
			    continue;

			if (line.trim().equals(""))
			    continue;

			HttpCookie cookie = readCookieRecord(line);

			// Store only non-expired cookie into in-memory cookie jar
			//
			if (cookie != null && cookie.hasExpired() == false)
			{
			    // Reject cookie if invalid for the cookie store
			    if (shouldRejectCookie(cookie)) 
				continue;

			    String domain = cookie.getDomain().toLowerCase();
			    ArrayList cookieList = (ArrayList) tempCookieJar.get(domain);
        
			    if (cookieList == null) 
				cookieList = new ArrayList();
        
			    if (addOrReplaceCookie(cookieList, cookie)) 
			    {
				tempCookieJar.put(domain, cookieList);
			    }
			}
		    }

		    br.close();
		    isr.close();
		    fis.close();

		    // Record last modified time of the cookie file 
		    lastAccessDate =  new Date(cookieFile.lastModified());

		    // Use new cookie jar only if reload is completely successful 
		    cookieJar = tempCookieJar;
		}
		catch (IOException ioe)
		{
		    ioe.printStackTrace();
		}
		catch (Throwable e)
		{
		    e.printStackTrace();
		}
	    }
	}
    }

    /**
     * Save cookie jar.
     */    
    protected void saveCookieJar()
    {
        AccessController.doPrivileged(new PrivilegedAction()
	{
    	    public Object run() 
	    {
		saveCookieJarToStorage();
		return null;
	    }
	});
    }

    /**
     * Save cookie jar into storage.
     */    
    private void saveCookieJarToStorage()
    {	
	try
	{
	    String lineSeparator = System.getProperty("line.separator");

	    StringBuffer sb = new StringBuffer();

	    sb.append("# Java Deployment HTTP Cookie File");
	    sb.append(lineSeparator);
	    sb.append("# http://www.netscape.com/newsref/std/cookie_spec.html");
	    sb.append(lineSeparator);
	    sb.append("# This is a generated file!  Do not edit.");
	    sb.append(lineSeparator);
	    sb.append(lineSeparator);


	    for (Iterator iter = cookieJar.values().iterator(); iter.hasNext(); )
	    {
		List cookieList = (List) iter.next();

		for (Iterator iter2 = cookieList.iterator(); iter2.hasNext(); )
		{
		    HttpCookie cookie = (HttpCookie) iter2.next();

		    if (cookie.hasExpired() == false)
		    {
			// Store only non-expired cookie into persist storage
			//
			writeCookieRecord(cookie, sb);
			sb.append(lineSeparator);
		    }
		}
	    }

	    // Create parent directories if absent
	    cookieFile.getParentFile().mkdirs();

	    // File I/O is done in one batch to reduce lock time of 
	    // the file.
	    //
	    FileOutputStream fos = new FileOutputStream(cookieFile);
	    BufferedOutputStream bos = new BufferedOutputStream(fos);
	    PrintWriter pw = new PrintWriter(bos);

	    pw.println(sb.toString());

	    pw.close();
	    bos.close();
	    fos.close();

	    // Record last modified time of the cookie file 
	    lastAccessDate = new Date(cookieFile.lastModified());
	}
	catch (IOException ioe)
	{
	    ioe.printStackTrace();
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}
    }


    /** 
     * Return cookie store name.
     */
    protected String getName()
    {
	return "Persistent Cookie Store";
    }


    /**
     * Predicate function which returns true if the cookie appears to be
     * invalid somehow and should not be added to the cookie set.
     */
    protected boolean shouldRejectCookie(HttpCookie cookie) 
    {
	// Call super
	if (super.shouldRejectCookie(cookie))
	    return true;

	// Persistent cookie has expiration date.and should not be expired
	if (cookie.getExpirationDate() == null || cookie.hasExpired())
	    return true;
	else
	    return false;
    }


    /**
     * Retrieve cookie from a record.
     *
     * @param line Cookie info
     * @return HTTP Cookie; null if cookie can not be retrieve
     */
    private HttpCookie readCookieRecord(String line)
    {
	// Line format is:
	//
	// host \t isDomain \t path \t secure \t expires \t name \t cookie
	//
	// if this format isn't respected we move onto the next line in the file.
	// isDomain is "TRUE" or "FALSE" (default to "FALSE")
	// isSecure is "TRUE" or "FALSE" (default to "TRUE")
	// expires is long integer (UNIX time)
	//
	// note: cookie can contain tabs.
        //

	try
	{
	    // When reading the cookie record, some field may contain empty string
	    // and is not recognizable by StringTokenizer. Thus, we need to parse the 
	    // delimiter manually.
	    //
	    StringTokenizer st = new StringTokenizer(line, "\t", true);

	    // Total number of normal fields including delimiters are 13. Three fields 
	    // (isDomain, path, isSecure) excludes delimiters are optional. Cookie 
	    // may also contains tab, so the minimial number of tokens is 10.
	    // 
	    if (st.countTokens() < 10)
	    {
		// Skip invalid cookie record
		return null;
	    }

	    // Domain that created AND that can read the cookie
	    String domain = st.nextToken();

	    // Domain must contains a "."
	    if (domain.indexOf(".") == -1)
		return null;

	    // Skip tab
	    st.nextToken();

	    // Flag indicates if all machines given a given domain can access the cookie
	    String flag = st.nextToken();

	    if (flag.equals("\t"))
	    {
		// Special case: empty string in the field, default to "FALSE"
		flag = "FALSE";
	    }
	    else
	    {
		// Skip tab
		st.nextToken();
	    }

	    // The path within the domain that cookie is valid for
	    String path = st.nextToken();

	    if (path.equals("\t"))
	    {
		// Special case: empty string in the field, default to "/"
		path = "/";
	    }
	    else
	    {
		// Skip tab
		st.nextToken();
	    }

	    // Flag indicates if a secure connection with the domain is needed to access the cookie.
	    String secure = st.nextToken();

	    if (secure.equals("\t"))
	    {
		// Special case: empty string in the field, default to "TRUE"
		secure = "TRUE";
	    }
	    else
	    {
		// Skip tab
		st.nextToken();
	    }
	    
	    boolean isSecure = !secure.equalsIgnoreCase("false");


	    // The UNIX time that the cookie will expire on.
	    Date expirationDate = new Date(new Long(st.nextToken()).longValue() * 1000);

	    // Skip tab
	    st.nextToken();

	    // Name of the cookie variable
	    String name = st.nextToken();
	    if (name.equals("\t") || name.trim().equals(""))
		return null;

	    // Skip tab
	    st.nextToken();

	    // Value of the cookie variable
	    String value = st.nextToken();

	    // Cookie may contains tab, so append the rest of the 
	    // tokens to form the final cookie value
	    while (st.hasMoreTokens())
		value += st.nextToken();

	    String nameAndValue = name + "=" + value;

	    return HttpCookie.create(expirationDate, nameAndValue, path, domain, isSecure);
	}
	catch (NoSuchElementException e)
	{
	    // Cookie parsing error
	    return null;
	}
	catch (NumberFormatException nfe)
	{
	    // Cookie parsing error
	    return null;
	}
    }
    
    /**
     * Persist information of the cookie into a buffer.
     *
     * @param cookie HTTP Cookie.
     * @param buffer Output string buffer.
     */
    private void writeCookieRecord(HttpCookie cookie, StringBuffer buffer)
    {
	String domain = cookie.getDomain();

	// Domain that created AND that can read the cookie
	buffer.append(domain);
	buffer.append("\t");

	// Flag indicates if all machines given a given domain can access the cookie
	if (domain.startsWith("."))
	    buffer.append("TRUE\t");
	else
    	    buffer.append("FALSE\t");

	// The path within the domain that cookie is valid for
	buffer.append(cookie.getPath());
	buffer.append("\t");
	
	// Flag indicates if a secure connection with the domain is needed to access the cookie.
	if (cookie.isSecure())
	    buffer.append("TRUE\t");
	else
    	    buffer.append("FALSE\t");

	// The UNIX time that the cookie will expire on.
	buffer.append(cookie.getExpirationDate().getTime() / 1000 + "\t");

	// Name of the cookie variable
	buffer.append(cookie.getName());
	buffer.append("\t");

	// Value of the cookie variable
	buffer.append(cookie.getValue());
    }
}

