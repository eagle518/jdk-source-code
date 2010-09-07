/*
 * @(#)HttpCookie.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.net.URL;
import java.util.Date;
import java.util.StringTokenizer;

/**
 * An object which represents an HTTP cookie.  Can be constructed by
 * parsing a string from the set-cookie: header.
 *
 * Syntax: Set-Cookie: NAME=VALUE; expires=DATE;
 *             path=PATH; domain=DOMAIN_NAME; secure
 *
 * All but the first field are optional.
 *
 */
class HttpCookie implements Comparable
{
    private static final int SIZE_LIMIT = 4096;

    private Date    expirationDate = null;
    private String  nameAndValue = null;
    private String  path = null;
    private String  domain = null;
    private boolean isSecure = false;

    /**
     * Construct a HttpCookie object.
     */
    private HttpCookie() 
    {
    }

    /**
     * Construct a HttpCookie object.
     */
    private HttpCookie(Date expirationDate, String nameAndValue, String path, String domain, boolean isSecure) 
    {
        this.expirationDate = expirationDate;
        this.nameAndValue   = nameAndValue;
        this.path           = path;
        this.domain         = stripPort(domain);
        this.isSecure       = isSecure;
    }

    private static String stripPort(String domainName) 
    {
        int index = domainName.indexOf(':');

        if (index == -1) 
	{
            return domainName;
        }

        return domainName.substring(0, index);
    }

    /**
     * Create a new HttpCookie object.
     */
    static HttpCookie create(Date expirationDate, String nameAndValue, String path, String domain, boolean isSecure)
    {
	// Check if cookie has required attributes.
	if (nameAndValue == null || path == null || domain == null)
	    return null;
	
	// Check if path is empty
	if (path.equals(""))
	    path = "/";

	// Truncate cookie if nameAndValue longer than 4K
	if (nameAndValue.length() > SIZE_LIMIT)
	    nameAndValue = nameAndValue.substring(0, SIZE_LIMIT);

	return new HttpCookie(expirationDate, nameAndValue, path, domain, isSecure);
    } 

    /**
     * Create a new HttpCookie object by parsing the given string into its individual 
     * components, recording them in the member variables of the object.
     */
    static HttpCookie create(URL url, String cookieString) 
    {
        StringTokenizer tokens = new StringTokenizer(cookieString, ";");

        if (!tokens.hasMoreTokens()) 
	{
            // make this robust against parse errors
	    return null;
        }

	HttpCookie httpCookie = new HttpCookie();
	
	httpCookie.nameAndValue = tokens.nextToken().trim();

	// Truncate cookie if nameAndValue longer than 4K
	if (httpCookie.nameAndValue.length() > SIZE_LIMIT)
	    httpCookie.nameAndValue = httpCookie.nameAndValue.substring(0, SIZE_LIMIT);

        while (tokens.hasMoreTokens()) 
	{
            String token = tokens.nextToken().trim();

            if (token.equalsIgnoreCase("secure")) 
	    {
                httpCookie.isSecure = true;
            } 
	    else 
	    {
                int equIndex = token.indexOf("=");

                if (equIndex < 0) 
		{
                    continue;

                    // malformed cookie
                }

                String attr = token.substring(0, equIndex);
                String val  = token.substring(equIndex + 1);

                if (attr.equalsIgnoreCase("path")) 
		{
                    httpCookie.path = val;
                } 
		else if (attr.equalsIgnoreCase("domain")) 
		{
                    if (val.indexOf(".") == 0) 
		    {
                        // spec seems to allow for setting the domain in
                        // the form 'domain=.eng.sun.com'.  We want to
                        // trim off the leading '.' so we can allow for
                        // both leading dot and non leading dot forms
                        // without duplicate storage.
                        httpCookie.domain = stripPort(val.substring(1));
                    } 
		    else 
		    {
                        httpCookie.domain = stripPort(val);
                    }
                } 
		else if (attr.equalsIgnoreCase("expires")) 
		{
                    httpCookie.expirationDate = parseExpireDate(val);
                } 
		else 
		{
                    // unknown attribute -- do nothing
                }
            }
        }

	// Fills in default values for domain, path, etc. from the URL
	// after creation of the cookie.
        //
	if (httpCookie.domain == null) 
	{
            httpCookie.domain = url.getHost();
        }

        if (httpCookie.path == null) 
	{
            httpCookie.path = url.getFile();

            // larrylf: The documentation for cookies say that the path is
            // by default, the path of the document, not the filename of the
            // document.  This could be read as not including that document
            // name itself, just its path (this is how NetScape intrprets it)
            // so amputate the document name!
            int last = httpCookie.path.lastIndexOf("/");

            if (last > -1) 
	    {
                httpCookie.path = httpCookie.path.substring(0, last);
            }
        }

	return httpCookie;
    }

    //======================================================================
    //
    // Accessor functions
    //
    public String getNameValue() 
    {
        return nameAndValue;
    }

    /**
     * Returns just the name part of the cookie
     */
    public String getName() 
    {
        int index = nameAndValue.indexOf("=");

        return nameAndValue.substring(0, index);
    }

    /**
     * Returns just the value part of the cookie
     */
    public String getValue() 
    {
        int index = nameAndValue.indexOf("=");

        return nameAndValue.substring(index + 1);
    }

    /**
     * Returns the domain of the cookie as it was presented
     */
    public String getDomain() 
    {
        return domain;
    }

    public String getPath() 
    {
        return path;
    }

    public Date getExpirationDate() 
    {
        return expirationDate;
    }

    boolean hasExpired() 
    {
        if (expirationDate == null) 
	{
            return false;
        }

        return (expirationDate.getTime() <= System.currentTimeMillis());
    }

    /**
     * Returns true if the cookie has an expiration date (meaning it's
     * persistent), and if the date nas not expired;
     */
    boolean isSaveable() 
    {
        return (expirationDate != null)
               && (expirationDate.getTime() > System.currentTimeMillis());
    }

    public boolean isSecure() 
    {
        return isSecure;
    }

    private static Date parseExpireDate(String dateString) 
    {
        // format is wdy, DD-Mon-yyyy HH:mm:ss GMT
        RfcDateParser parser  = new RfcDateParser(dateString);
        Date          theDate = parser.getDate();

        return theDate;
    }

    public String toString() 
    {
        String result = nameAndValue;

        if (expirationDate != null) 
	{
            result += "; expires=" + expirationDate;
        }

        if (path != null) 
	{
            result += "; path=" + path;
        }

        if (domain != null) 
	{
            result += "; domain=" + domain;
        }

        if (isSecure) 
	{
            result += "; secure";
        }

        return result;
    }

    /**
     * Compare this HttpCookie object to the other.
     */
    public int compareTo(Object o)
    {
	HttpCookie other = (HttpCookie) o;

	// Compare self
	if (other == this)
	    return 0;

	// Compare domain
	int c = getDomain().compareTo(other.getDomain());

	if (c != 0)
	    return c;

	// Compare path
	c = getPath().compareTo(other.getPath());

	if (c != 0)
	    return c;

	// Compare name/value pair
	return getNameValue().compareTo(other.getNameValue());
    }  
}

