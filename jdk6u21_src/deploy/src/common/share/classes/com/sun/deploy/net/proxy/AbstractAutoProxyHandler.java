/*
 * @(#)AbstractAutoProxyHandler.java	1.9 10/03/24 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.Reader;
import java.io.StringWriter;
import java.net.InetAddress;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.StringTokenizer;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for auto proxy configuration.
 */
public abstract class AbstractAutoProxyHandler implements ProxyHandler 
{
    // Browser proxy info
    private BrowserProxyInfo bpi = null;

    // JavaScript for auto proxy config
    protected StringBuffer autoProxyScript = null;

    // JavaScript from pac/js file
    protected String jsPacScript = null;

    /**
     * Check if the proxy handler supports the proxy type
     *
     * @param proxyType Proxy type
     * @return true if proxy type is supported
     */
    public final boolean isSupported(int proxyType)
    {
	return (proxyType == ProxyType.AUTO);
    }

    /**
     * Check if the proxy result should be cached
     *
     * @return true if proxy result should be cached
     */
    public final boolean isProxyCacheSupported()
    {
	return true;
    }

    /**
     * Return true if auto proxy is handled by IE.
     */
    protected abstract boolean isIExplorer();

    /**
     * Initialize the auto proxy handler.
     *
     * @param info Browser proxy info
     */
    public final void init(BrowserProxyInfo info) 
			   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.loading.auto");

	// Check if proxy type is supported
	if (isSupported(info.getType()) == false)
	    throw new ProxyConfigException("Unable to support proxy type: " + info.getType());

	// Store browser proxy info
	bpi = info;

	// Construct the JavaScript
	//
	autoProxyScript = new StringBuffer();

	// Combine Auto proxy script
	autoProxyScript.append(AutoProxyScript.jsGlobal);
	autoProxyScript.append(AutoProxyScript.jsDnsDomainIs);
	autoProxyScript.append(AutoProxyScript.jsIsPlainHostName);
	autoProxyScript.append(AutoProxyScript.jsconvert_addr);

	// Check the browser type for different implementation of dnsResolve 
	// and isInet function;
	// Fix for 4670449 (5/9/02)
	//
	if (isIExplorer())
	{
	    autoProxyScript.append(AutoProxyScript.jsIsInNetForIE);
	    autoProxyScript.append(AutoProxyScript.jsDnsResolveForIE);
	}
	else
	{
	    autoProxyScript.append(AutoProxyScript.jsIsInNetForNS);
	    autoProxyScript.append(AutoProxyScript.jsDnsResolveForNS);
	}

	autoProxyScript.append(AutoProxyScript.jsIsResolvable);
	autoProxyScript.append(AutoProxyScript.jsLocalHostOrDomainIs);
	autoProxyScript.append(AutoProxyScript.jsDnsDomainLevels);
	autoProxyScript.append(AutoProxyScript.jsMyIpAddress_0);

	try
	{
	    InetAddress address = InetAddress.getLocalHost();
	    autoProxyScript.append(address.getHostAddress());
	}
	catch (Throwable e)
	{
	    e.printStackTrace();

	    // If somehow we fail to obtain the IP address,
	    // use loop back
	    autoProxyScript.append("127.0.0.1");
	}

	autoProxyScript.append(AutoProxyScript.jsMyIpAddress_1);
	autoProxyScript.append(AutoProxyScript.jsShExpMatch);
	autoProxyScript.append(AutoProxyScript.jsEnableDateRange);
	autoProxyScript.append(AutoProxyScript.jsEnableTimeRange);
	autoProxyScript.append(AutoProxyScript.jsEnableWeekdayRange);


	// Download the auto proxy config file if necessary
	//
	URL autoConfigURL = null;
	try
	{
	    autoConfigURL = new URL(bpi.getAutoConfigURL());
	}
	catch (MalformedURLException e)
	{
	    throw new ProxyConfigException("Auto config URL is malformed");
	}

	// If we are here, the URL should be okay
	
	// Check if auto proxy file is INS file supported by IEAK
	if (autoConfigURL.getFile().toLowerCase().endsWith(".ins"))
	{
	    // Try to download the IEAK file and parse the AutoConfigJSURL field
	    jsPacScript = getJSFileFromURL(getAutoConfigURLFromINS(autoConfigURL));
	}
	else
        {
	    // If we are here, the URL should be a JavaScript file
	    jsPacScript = getJSFileFromURL(autoConfigURL);
	}
	
	autoProxyScript.append(jsPacScript);

	Trace.msgNetPrintln("net.proxy.loading.done");
    }


    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public abstract ProxyInfo[] getProxyInfo(URL u)
	throws com.sun.deploy.net.proxy.ProxyUnavailableException;


    /**
     * Open a connection to the URL specified with no proxy
     *
     * @param URL url
     * @return URLConnection object
     */
    private URLConnection getDirectURLConnection(URL url)
	throws ProxyConfigException
    {
	URLConnection conn = null;

	if (url == null) return null;
	
	try {
	    // Obtain protocol
	    String protocol = url.getProtocol();
	    
	    if (protocol.equals("file")) {
		// Canonicalize the file URL if necessary;
		String urlStr = url.toExternalForm();
		int i = urlStr.indexOf('/');
		if (i == -1)
		  throw new ProxyConfigException("Malformed URL specified:" + url); 
		while (urlStr.charAt(++i) == '/');
		    
		URL canonicalizedURL = new URL("file:/" + urlStr.substring(i));
		
		conn = canonicalizedURL.openConnection();
	    }
	    else {
		// Create a URL connection without proxy
		try {
		    conn = url.openConnection(java.net.Proxy.NO_PROXY);
		} catch (NoClassDefFoundError e) {
		    // in webstart, pre tiger, no proxies set up yet, and no
		    // java.net.Proxy class exists
		    conn = url.openConnection();
		}
	    }
	} 
	catch(IOException e) {
	    throw new ProxyConfigException("Unable to obtain a connection from " + url, e);
	}
	
	return conn;
    }

    /**
     * Download INS file for Internet Explorer Administration Kit (IEAK), 
     * and obtain the AutoConfigURL from there.
     *
     * @param URL url to INS file
     * @return Auto config URL in INS file
     */
    private URL getAutoConfigURLFromINS(URL url)
		   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.auto.download.ins", new Object[] {url});

	String autoConfigURL = null;

	try {
	    URLConnection conn = getDirectURLConnection(url);

	    // Add Comment filter
	    BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
	    String buffer = null;

	    do
	    {
		buffer = br.readLine();

		if (buffer != null)
		{
		    // It is VERY important to check if the index
		    // is 0 because it is only valid if AutoConfigJSURL is
		    // not commented out by users.
		    //
		    if (buffer.indexOf("AutoConfigJSURL=") == 0)
		    {
			autoConfigURL = buffer.substring(16);
			break;
		    }
		}		    
	    }
	    while (buffer != null);

	    // Close the streams
	    br.close();

	    if (autoConfigURL != null) 
		return new URL(autoConfigURL);
	    else 
		throw new ProxyConfigException("Unable to locate 'AutoConfigJSURL' in INS file");
	}
	catch (ProxyConfigException e0)
	{   
	    throw e0;
	}
	catch (MalformedURLException e1)
	{
	    throw new ProxyConfigException("Malformed URL specified in INS file: " + autoConfigURL, e1);
	} 
	catch (Throwable e2)
	{   
	    throw new ProxyConfigException("Unable to obtain INS file from " + url, e2);
	}
    }

    /**
     * getJSFilefromURL downloads a JS file through HTTP with no proxy, and return
     * the file content.
     **/
    private String getJSFileFromURL(URL url)
		   throws ProxyConfigException
    {
	Trace.msgNetPrintln("net.proxy.auto.download.js", new Object[] {url});

        try  {

	    URLConnection conn = getDirectURLConnection(url);

            // Add Comment filter
	    Reader rin = new RemoveCommentReader(new InputStreamReader(conn.getInputStream()));
	    BufferedReader br = new BufferedReader(rin);
	    StringWriter sw = new StringWriter();

	    char[] buffer = new char[4096];
	    int numchars;
	    while ((numchars = br.read(buffer)) != -1)
	    {
		sw.write(buffer, 0, numchars);
	    }

	    // Close the streams
	    br.close();
	    rin.close();
	    sw.close();

	    return sw.toString();
        }
        catch (Throwable e) 
	{
	    throw new ProxyConfigException("Unable to obtain auto proxy file from " + url, e);
        }
    }
   

    /**
     * extractAutoProxySetting is a function which takes a proxy-info-string
     * which returned from the JavaScript function FindProxyForURL, and returns
     * the corresponding proxy information.
     *
     * parameters :
     *	s         [in]	a string which contains all the proxy information
     *
     * out:
     *   ProxyInfo [out] ProxyInfo contains the corresponding proxy result.
     *
     * Notes: i) s contains all the proxy information in the form of
     *        "PROXY webcache1-cup:8080;SOCKS webcache2-cup". There are three
     *        possible values inside the string:
     *	        a) "DIRECT" -- no proxy is used.
     *          b) "PROXY"  -- Proxy is used.
     *          c) "SOCKS"  -- SOCKS support is used.
     *        Information for each proxy settings are seperated by ';'. If a
     *	      port number is specified, it is specified by using ':' following
     *        the proxy host.
     *
     */
    protected final ProxyInfo[] extractAutoProxySetting(String s)
    {
	if (s != null)
	{
	    StringTokenizer st = new StringTokenizer(s, ";", false);
	    ProxyInfo proxyInfoArray[] = new ProxyInfo[st.countTokens()];
	    int index = 0;
	    
	    while (st.hasMoreTokens()) 
	    {  
		String pattern = st.nextToken();     

		int i = pattern.indexOf("PROXY");

		if (i != -1)   {
		    // "PROXY" is specified
		    proxyInfoArray[index++] = new ProxyInfo(pattern.substring(i + 6));
		    continue;
		}

		i = pattern.indexOf("SOCKS");

		if (i != -1) 
		{
		    // "SOCKS" is specified
		    proxyInfoArray[index++] = new ProxyInfo(null, pattern.substring(i + 6));
		    continue;
		}
                // proxy string contains 'DIRECT' or unrecognized text
		proxyInfoArray[index++] = new ProxyInfo(null, -1);
	    }
	    return proxyInfoArray;
	}
	//In order to make the return value safe to use, created a null ProxyInfo object
	return new ProxyInfo[] {new ProxyInfo(null)};
    }
}



