/*
 * @(#)WDefaultBrowserProxyConfig.java	1.28 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
	if (browser != null) {
	    String browserLowerCase = browser.toLowerCase();

            // look for Firefox prefs.js
	    if (browserLowerCase.indexOf("firefox") != -1) {
		Trace.println("Browser is Firefox", TraceLevel.NETWORK);
                WFirefoxProxyConfig firefoxProxyConfig = new WFirefoxProxyConfig();
		BrowserProxyInfo info = firefoxProxyConfig.getBrowserProxyInfo();
                if (info.getType() != ProxyType.UNKNOWN) {
                    return info;
                }
            }

            // Fallback to Netscape 6
	    if (browserLowerCase.indexOf("netscape") != -1 ||
	     browserLowerCase.indexOf("netscp") != -1 ||
	     browserLowerCase.indexOf("mozilla") != -1 ) {
		Trace.println("Browser is Netscape/Mozilla", 
			      TraceLevel.NETWORK);
		float version = getNSVersion();
	    
		Trace.println("version: " + version, TraceLevel.NETWORK);
                
		// mozilla and netscape 6+ shares the same prefs.js file
		// for storing proxy information
		if (version >= 6 || browserLowerCase.indexOf("mozilla") != -1)
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
                BrowserProxyInfo info = ns4ProxyConfig.getBrowserProxyInfo();
                
                // If browser settings were found use them, otherwise fall 
                // through to use the IE settings
                if (info != null) {
                    return info;
                }
	    }
	}
	    
	// Defaults to use IE settings if Netscape is not default browser
        // or no valid Netscape browser settings file could be found
        
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

    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }
    
}
