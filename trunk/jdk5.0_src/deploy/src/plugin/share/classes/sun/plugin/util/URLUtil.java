/*
 * @(#)URLUtil.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.net.URL;
import java.util.WeakHashMap;


/*
 * URL Canonicalizer is used for canonicalizing weird/malformed URL
 * that are recognized by the browsers but not Java Plug-in.
 *
 * @author	Stanley Man-Kit Ho
 *
 */
public class URLUtil
{
    // Use a map to store canonicalized URL for performance
    // reasons.
    private static WeakHashMap canonicalizedURLMap = new WeakHashMap();

    /**
     * Canonicalize URL
     */
    public synchronized static String canonicalize(String url)
    {
	/*
	 * Force the docbase for Internet Explorer mounted drive
	 * applet display. In IE, when displaying html pages from
	 * a mounted the drive, the document base will not be
	 *
	 *	    file://<drive>:\...
	 * but
	 *	    file://///<server>\<mount_name>\...
	 *	    file:///\\<server>\<mount_name>\...
	 *  	    file://<server>/<mount_name>\...
	 *  	    file:\\<server>\<mount_name>\...
	 *
	 * which causes the failure to load ressources or classes from
	 * a jar file. We must mangle the URL to the correct setting
	 *
	 * In the other cases, somehow IE will pass us a bogus file 
	 * URL in the form of
	 *	    file://C:\directory\file.html
	 *
         * We MUST convert it into proper form
	 */

	if (url.indexOf("file:") == -1)
	{
	    // Do nothing if this is not a file URL
	    return url;
	}    


	// Obtain canonicalized URL from map only if it is
	// file URL
	// 
	String value = (String) canonicalizedURLMap.get(url);
    
	if (value != null)
	    return value;


	// Begin canonicalization
	//
	StringBuffer urlString = new StringBuffer();

	if (url.indexOf("file://///") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file://///<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(10));
	}
	else if (url.indexOf("file:///\\") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file:///\\<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(9));
	}
	else if (url.indexOf("file://\\") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file://\\<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(9));
	}
	else if (url.indexOf("file:\\") == 0)
	{
	    // Check if the URL is in the form
	    //
	    //	    file:\\C:\directory\file.html
	    // or
	    // 	    file:\\<server>\<mount_name>\...
	    //

	    if (url.indexOf(':', 6) != -1 || url.indexOf('|', 6) != -1)
		urlString.append("file:///");
	    else
		urlString.append("file:////");

	    urlString.append(url.substring(6));
	}
	else if (url.indexOf("file://") == 0 &&
	         url.charAt(7) != '/')			    // skip "file:///C:/..."
	{	    
	    // Check if the URL is in the form
	    //
	    //	    file://C:\directory\file.html
	    // or
	    // 	    file://<server>/<mount_name>\...
	    //

	    if (url.indexOf(':', 7) != -1 || url.indexOf('|', 7) != -1)
		urlString.append("file:///");
	    else
		urlString.append("file:////");

	    urlString.append(url.substring(7));
	}
        else
	{
	    urlString.append(url);
	} 
	              
        // Search for '\' and replace it with '/'
	// Search for first '|' and replace it with ':'
	//
	boolean found = false;

        for (int i=0; i < urlString.length(); i++)
        {
	    char c = urlString.charAt(i);

            if (c == '\\')
	    {
                urlString.setCharAt(i, '/');
	    }
	    else if (!found && c == '|')
	    {
		// only replace the first "|"
		urlString.setCharAt(i, ':');
		found = true;
	    }
        }

	// Store canonicalized URL into map
	//
	String canonicalizedURL = urlString.toString();

	canonicalizedURLMap.put(url, canonicalizedURL);

	return canonicalizedURL;
    }

/*
    private static void test()
    {
	System.out.println("Normal URL");
	System.out.println(URLUtil.canonicalize("file:/C:\\Document\\Temp"));
	System.out.println(URLUtil.canonicalize("file:///C:\\Document\\Temp"));
	System.out.println("\nAbnormal URL");
	System.out.println(URLUtil.canonicalize("file://///xpjava\\Document\\Temp"));
	System.out.println(URLUtil.canonicalize("file:///\\xpjava\\Document\\Temp"));
	System.out.println(URLUtil.canonicalize("file:\\xpjava\\Document\\Temp"));
	System.out.println(URLUtil.canonicalize("file://xpjava/Document/Temp"));
	System.out.println(URLUtil.canonicalize("file://C:\\Document\\Temp"));
	System.out.println(URLUtil.canonicalize("file:\\C:\\Document\\Temp"));
    }
*/
}
