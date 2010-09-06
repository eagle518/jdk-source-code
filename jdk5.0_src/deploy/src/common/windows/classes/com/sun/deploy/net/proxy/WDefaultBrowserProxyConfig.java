/*
 * @(#)WDefaultBrowserProxyConfig.java	1.22 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import com.sun.deploy.util.WinRegistry;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.Trace;


/**
 * Looks up whatever information is available about the default
 * HTTP proxy server address.  For IE this means consulting a few
 * registry entries, for Netscape this means finding the JavaScript
 * preferences file and looking up the address information there.
 *
 * @version 1.7, 02/09/01
 */
public class WDefaultBrowserProxyConfig implements BrowserProxyConfig 
{
    /**
     * Return all of the available information about the internet proxy addresses.
     * This is as simple as looking up a few registry entries for Internet Explorer,
     * it's a fairly convoluted process for Netscape.
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	/* Determine if the default browser is NS4.x or IE.  The registry key
	 * used for this is documented here:
	 *    http://help.netscape.com/kb/consumer/19980502-2.html
	 */
	String appPath = "http\\shell\\open\\command";
	int appKey = WinRegistry.HKEY_CLASSES_ROOT;
	String browser = WinRegistry.getString(appKey, appPath, "");
	
	Trace.println("Browser is " + browser, TraceLevel.NETWORK);
	
	/* Netscape
	 *
	 * All of the interestring Navigator information is stored in a private
	 * binary registry file, see the NSPreferences class for more information.
	 */
	if ((browser != null) &&
	    (browser.toLowerCase().indexOf("netscape") != -1 ||
	     browser.toLowerCase().indexOf("netscp") != -1 )) {
	    Trace.println("Browser is Netscape", TraceLevel.NETWORK);
	    float version = getNSVersion();
	    
	    Trace.println("version: " + version, TraceLevel.NETWORK);

	    if (version >= 6) 
	    {
		WNetscape6ProxyConfig ns6ProxyConfig = new WNetscape6ProxyConfig();

		// Retrieve Netscape 6 proxy info
		BrowserProxyInfo info = ns6ProxyConfig.getBrowserProxyInfo();

		if (info.getType() != ProxyType.UNKNOWN)
		    return info;
	    }

	    // Fallback to Netscape 4
	    WNetscape4ProxyConfig ns4ProxyConfig = new WNetscape4ProxyConfig();

	    // Retrieve Netscape 4 proxy info
	    return ns4ProxyConfig.getBrowserProxyInfo();
	}
	    
	// Defaults to use IE settings if Netscape is not default browser

	/* Internet Explorer
	 *
	 * The Proxy{Server,Override,Enable} registry keys are covered by several
	 * MS web pages however only as a sidebar.  Here's an example:
	 *   http://support.microsoft.com/support/kb/articles/Q164/0/35.ASP
	 */
	
	Trace.println("Browser is IE", TraceLevel.NETWORK);

	WIExplorerProxyConfig ieProxyConfig = new WIExplorerProxyConfig();

	// Retrieve Internet Explorer proxy info
	return ieProxyConfig.getBrowserProxyInfo();
    }

    /**
     * Returns the NS Navigator version as a float if it was possible
     * to parse the string, -1.0 otherwise.  A typical version string
     * is "4.61 (en)".
     */
    private static float getNSVersion() {
	// NS 6.x
	String path = "Software\\Netscape\\Netscape 6";
	String s = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE, path, "CurrentVersion");

	// NS 4.x
	if (s == null) {
	    path = "Software\\Netscape\\Netscape Navigator";

	    s = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE, path, "CurrentVersion");

	}

	Trace.println("NS version string: " + s, TraceLevel.NETWORK);

	// no NS version found
	if (s == null) return -1.0f;
	
	int start = -1, end = -1;
	boolean dot = false;
	/* Find the beginning and the end of the version number. */
	for(int i = 0; (i < s.length()) && (end == -1); i++) {
	    char c = s.charAt(i);
	    // 6.x.y -> 6.x
	    if (c == '.' && start != -1 && dot == true) {
		end = i;
		break;
	    }
	    if (c == '.') dot = true;
	    boolean isFloatChar = Character.isDigit(c) || (c == '.');
	    if (start == -1) {
		if (isFloatChar) {
		    start = i;
		}
	    }
	    else if (!isFloatChar) {
		end = i;
	    }


	}
	/* Parse */
	if ((start != -1) && (end > start)) {
	    try {
		
		return Float.parseFloat(s.substring(start, end));
	    }
	    catch (NumberFormatException e) {
		return -1.0f;
	    }
	}
	else {	    
	    return -1.0f;
	}
    }     
}
