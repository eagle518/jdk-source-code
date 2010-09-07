/*
 * @(#)WNetscape4ProxyConfig.java	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.File;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.WinRegistry;


/**
 * Proxy configuration for Netscape Navigator 4 on Win32.
 */
public final class WNetscape4ProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     * @return the browser settings, or null if they were not retrieved
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ns");
	
	BrowserProxyInfo info = new BrowserProxyInfo();

	float version = getNSVersion();
	File file = null;
	
	if (version >= 4.5) 
	{
	    String windowsDir = WinRegistry.getWindowsDirectory();

	    if (windowsDir != null) {
		file = getNSPrefsFile(new File(windowsDir, "nsreg.dat"));
	    }
	}
	else 
	{
	    file = getNS40PrefsFile();
	}
	
	if (file != null) 
	{
	    // Display user preference file location
	    Trace.msgNetPrintln("net.proxy.browser.pref.read", new Object[] {file.getPath()});

	    NSPreferences.parseFile(file, info, version, true);
	} else {
	    info = null;
        }
        
	Trace.msgNetPrintln("net.proxy.loading.done");

	return info;
    }

    /**
     * Returns the NS Navigator version as a float if it was possible
     * to parse the string, -1.0 otherwise.  A typical version string
     * is "4.61 (en)".
     */
    private static float getNSVersion() 
    {
	String path = "Software\\Netscape\\Netscape Navigator";

	String s = WinRegistry.getString(WinRegistry.HKEY_LOCAL_MACHINE, path, "CurrentVersion");

	int start = -1, end = -1;

        // If valid browser version key was found
        if ( s != null ) {
            /* Find the beginning and the end of the version number. */
            for(int i = 0; (i < s.length()) && (end == -1); i++) {
                char c = s.charAt(i);
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
     * Return the location of the "prefs.js" user profile file in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 4.5 - 4.7 of Navigator.
     */
    static File getNSPrefsFile(File registryFile) 
    {
	NSRegistry reg = new NSRegistry().open(registryFile);
	String path = null;

	// Get current user profile directory
	if (reg != null) 
	{
	    String user = reg.get("Common/Netscape/ProfileManager/LastNetscapeUser");
	    if (user != null) {
		path = reg.get("Users/" + user + "/ProfileLocation");
	    }
	    reg.close();
	}

	return (path != null) ? new File(path, "prefs.js") : null;
    }
    
    
    /**
     * Return the location of the "prefs.js" user profile file or null
     * if we can't figure that out.  The directory that contains this file
     * is found under this registry entry:
     * <pre>
     *     Software\Netscape\Netscape Navigator\Users\<CurrentUser>\DirRoot
     * </pre>
     * This method should work with versions 4.0.x of Navigator.
     */
    private static File getNS40PrefsFile() 
    {
	String path = null;
	int key = WinRegistry.HKEY_LOCAL_MACHINE;

	// Get current user name
	String usersPath = "Software\\Netscape\\Netscape Navigator\\Users";
	String currentUser = WinRegistry.getString(key, usersPath, "CurrentUser");

	// Get current user directory
	if (currentUser != null) 
	{
	    String userPath = "Software\\Netscape\\Netscape Navigator\\Users\\" + currentUser;
	    path = WinRegistry.getString(key, userPath, "DirRoot");
	}
	return (path != null) ? new File(path, "prefs.js") : null;
    }
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }
    
}




