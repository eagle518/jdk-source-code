/*
 * @(#)ManualProxyHandler.java	1.24 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;
import java.net.URL;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for manual proxy configuration.
 */
final class ManualProxyHandler implements ProxyHandler 
{
    // Browser proxy info
    private BrowserProxyInfo bpi = null;	

    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public boolean isSupported(int proxyType)
    {
	return (proxyType == ProxyType.MANUAL);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public boolean isProxyCacheSupported()
    {
	return true;
    }

    /**
     * Initialize the direct proxy handler.
     *
     * @param info Browser proxy info
     */
    public void init(BrowserProxyInfo info)
		throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.manual");
	
	// Check if proxy type is supported
	if (isSupported(info.getType()) == false)
	    throw new ProxyConfigException("Unable to support proxy type: " + info.getType());

	bpi = (BrowserProxyInfo) info;

	proxyOverridePatterns = new ArrayList();

	// Enumerate the override list and determine the patterns
	String[] list = bpi.getOverrides();

	if (list != null)
	{
	    Trace.msgNetPrintln("net.proxy.pattern.convert");

	    for (int i=0; list != null && i < list.length; i++)
	    {
		String item = list[i];

		try
		{
		    if (item.equals("<local>"))
		    {
			// Insert special pattern if by-pass all local host
			proxyOverridePatterns.add(Pattern.compile("[^.]+"));

			Trace.netPrintln("    <local> --> [^.]+");
		    }
		    else
		    {
			// Canonicalize the pattern
			Pattern pattern = canonicalizePattern(item);

	    		proxyOverridePatterns.add(pattern);

			Trace.netPrintln("    " + item + " --> " + pattern.pattern());
		    }
		}
		catch (PatternSyntaxException e)
		{
		    e.printStackTrace();

		    Trace.msgNetPrintln("net.proxy.bypass.convert.error");
		}
	    }
	}

	Trace.msgNetPrintln("net.proxy.loading.done");
    }


    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
    {
	String protocol = u.getProtocol();
	String host = u.getHost();

	// Check if proxy is by-passed
	if (isProxyOverriden(host.toUpperCase()))
	{
	    // by-pass -> no proxy
	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
	else
	{
	    // Should use proxy

	    String proxyHost = null;
	    int proxyPort = -1;

	    if (protocol.equals("http"))
	    {
		proxyHost = bpi.getHttpHost();
		proxyPort = bpi.getHttpPort();
	    }	    
	    else if (protocol.equals("https"))
	    {
		proxyHost = bpi.getHttpsHost();
		proxyPort = bpi.getHttpsPort();
	    }	    
	    else if (protocol.equals("ftp"))
	    {
		proxyHost = bpi.getFtpHost();
		proxyPort = bpi.getFtpPort();
	    }	    
	    else if (protocol.equals("gopher"))
	    {
		proxyHost = bpi.getGopherHost();
		proxyPort = bpi.getGopherPort();
	    }	   
	    return new ProxyInfo[] {new ProxyInfo(proxyHost, proxyPort, bpi.getSocksHost(), bpi.getSocksPort())};
	}
    }


    /**
     * Canonicalize pattern into regular expression.
     *
     * @param pattern Pattern to be canonicalized
     * @return Canonicalized pattern
     */
    private Pattern canonicalizePattern(String pattern)
		    throws PatternSyntaxException
    {
	if (pattern == null)
	    return null;

	StringBuffer buffer = new StringBuffer();

	// Convert all characters to upper case and in regular 
	// expression form
	for (int i=0; i < pattern.length(); i++)
	{
	    char c = pattern.charAt(i);

	    // Convert character into regular expression
	    if (c == '*')
		buffer.append(".*");
	    else
		buffer.append(Character.toUpperCase(c));
	}

	return Pattern.compile(buffer.toString());
    }


    // List that stores all the patterns of the bypass list.
    private List proxyOverridePatterns = null;

    /**
     * Check if the proxy host is in the bypass list.
     * 
     * @param host Host name
     * @return true if the proxy host should be bypassed
     */
    private boolean isProxyOverriden(String host)
    {
	// Iterate through the entire pattern list
	Iterator iter = proxyOverridePatterns.iterator();

	while (iter.hasNext())
	{	    
	    Pattern pattern = (Pattern) iter.next();
	    Matcher matcher = pattern.matcher(host);
	    
	    if (matcher.matches())
		return true;
	}

	return false;
    }

/*
    public static void test()
    {	
	try
	{
	    System.out.println(Pattern.compile("[^.]+").matcher("a").matches());
	    System.out.println(Pattern.compile("[^.]+").matcher("a.b").matches());
	    System.out.println(Pattern.compile("[^.]+").matcher("a.b.c").matches());
	    System.out.println(Pattern.compile("a.b.c").matcher("a.b.c").matches());
	    System.out.println(Pattern.compile("a.b.c.*").matcher("a.b.c").matches());
	    System.out.println(Pattern.compile("a.b..*").matcher("a.b.c").matches());
	    System.out.println(Pattern.compile("a.c..*").matcher("a.b.c").matches());
	    System.out.println(Pattern.compile("www..*.com").matcher("www.sun.com").matches());
	    System.out.println(Pattern.compile("www..*.1com").matcher("www.sun.com").matches());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}
    }
*/
}



