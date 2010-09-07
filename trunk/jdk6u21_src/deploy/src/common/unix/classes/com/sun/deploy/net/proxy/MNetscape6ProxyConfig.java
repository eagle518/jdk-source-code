/*
 *  @(#)MNetscape6ProxyConfig.java	1.11 04/11/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.File;
import java.io.IOException;
import com.sun.deploy.util.Trace;


/**
 * Proxy configuration for Netscape Navigator 6 on Unix
 */
public final class MNetscape6ProxyConfig implements BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ns");
	
	BrowserProxyInfo info = new BrowserProxyInfo();

	// Determine home directory
	try 
	{
	    String homeDir = System.getProperty("user.home");
	    
	    File regFile = new File(homeDir + "/.mozilla/appreg");
	    
	    File file = null;
	    try {
	        file = NSPreferences.getNS6PrefsFile(regFile);
                Trace.msgNetPrintln("net.proxy.browser.pref.read", new Object[] {file.getPath()});
                NSPreferences.parseFile(file, info, 6, true);
                
                // If User has selected Auto Proxy detection determine 
                // what the correct WPAD URL should be.  
                if( info.isAutoProxyDetectionEnabled() ) {
                    
                    // Set the Auto Config URL to the WPAD based URL
                    info.setAutoConfigURL( WebProxyAutoDetection.getWPADURL() );
                }
                
	    }
	    catch (IOException e)
	    {
		Trace.msgNetPrintln("net.proxy.ns6.regs.exception", new Object[] {regFile.getPath()});
		info.setType(ProxyType.UNKNOWN);
	    }
	}
	catch (SecurityException e) 
	{
    	    Trace.netPrintException(e);
	    info.setType(ProxyType.UNKNOWN);
	}
        
	// This is a workaroud for NS6 because of the LiveConnect bug
	//
	if (java.security.AccessController.doPrivileged(
	      new sun.security.action.GetPropertyAction("javaplugin.version")) != null)
        {
            info.setType(ProxyType.BROWSER);
	}
	Trace.msgNetPrintln("net.proxy.loading.done");
	
	return info;
    }
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }
    
}
