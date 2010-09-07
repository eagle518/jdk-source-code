/*
 * @(#)WIExplorerProxyConfig.java	1.23 04/11/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.WinRegistry;


/**
 * Proxy configuration for Internet Explorer on Win32.
 */
public final class WIExplorerProxyConfig implements BrowserProxyConfig 
{
    // Constant defined in MSDN
    private static final String REGSTR_PATH_INTERNET_SETTINGS = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    private static final String REGSTR_PATH_AUTOPROXY_DETECT = 
            REGSTR_PATH_INTERNET_SETTINGS + "\\Connections";
    private static final String REGSTR_VAL_PROXYENABLE = "ProxyEnable";
    private static final String REGSTR_VAL_PROXYSERVER = "ProxyServer";
    private static final String REGSTR_VAL_PROXYOVERRIDE = "ProxyOverride";
    private static final String REGSTR_VAL_AUTOCONFIGURL = "AutoConfigURL";
    private static final String DEFAULT_CONNECTION_SETTINGS = "DefaultConnectionSettings";
                                
    /*
     * Windows API will return the Auto Proxy URL on a WPAD enabled network
     */
    private native String performAutoDetection();
    
    /* 
     * Returns browser proxy info
     */
    public BrowserProxyInfo getBrowserProxyInfo()
    {
	Trace.msgNetPrintln("net.proxy.loading.ie");

	BrowserProxyInfo info = new BrowserProxyInfo();
	info.setType(ProxyType.NONE);

	ProxyInfo proxyInfo = getBrowserProxySettings();
        
	if(proxyInfo != null) {
	    switch(proxyInfo.getProxyType()) {
	    case 1:		// direct connection
		info.setType(ProxyType.NONE);
		break;
	    case 3:		// via proxy
		info.setType(ProxyType.MANUAL);
		if(proxyInfo.getProxy() != null) {
		    String proxy = proxyInfo.getProxy().replace(' ', ';');
		    ProxyUtils.parseProxyServer(proxy, info);
		}
		if(proxyInfo.getProxyBypass() != null) {
		    StringTokenizer st = new StringTokenizer(proxyInfo.getProxyBypass(), " ;");
		    String token;
		    ArrayList list = new ArrayList();
		    while(st.hasMoreTokens()) {
			token = st.nextToken().toLowerCase(java.util.Locale.ENGLISH).trim();
			if(token != null && token.length() > 0) {
			    list.add(token);
			}
		    }
		    info.setOverrides(list);
		}

		break;
	    default:
		info.setType(ProxyType.NONE);
	    }
	}  
        
	// Check if auto Proxy Detection is enabled if it is, 
        // the URL it returns will be used
        // If it is unable to find a valid script, check to see 
        // if the user manually entered one
        if( isAutoDetectEnabled() ) {
            info.setAutoProxyDetectionEnabled( true );
            // Get the URL from Auto-Proxy Detection
            info.setAutoConfigURL( performAutoDetection() ); 
        } 
        
        // If WPAD isn't enabled, or no valid URL was found
        // check to see if the location was manually entered
	if ( info.getAutoConfigURL() == null ) { 
	    // Try to the URL that was manually entered 
	    info.setAutoConfigURL( getAutoConfigURL() ); 
	}
        
        // If we have an Auto Proxy URL set the type to AUTO
        if(  info.getAutoConfigURL() != null ) {
            
            Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", 
                    new Object[] { info.getAutoConfigURL() });
            
            // Proxy type is Auto as a script was located to use
            info.setType(ProxyType.AUTO);
        }
        
	Trace.msgNetPrintln("net.proxy.loading.done");	
        
	return info;
    }
    
    /**
     * retrives the location of the auto-configuration script if one is being
     * used
     * @return The location of the configuration script (file or URL)
     */
    public String getAutoConfigURL( ) {
 
        String scriptLocation = null;
        
        // Pull from the Reg the Autoproxy Location
        scriptLocation = WinRegistry.getString(WinRegistry.HKEY_CURRENT_USER, 
                REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_AUTOCONFIGURL); 
        
        return scriptLocation;
    }
    
    /**
     * Determins if Auto Proxy detection has been enabled
     * 
     * @return true if Auto Proxy detection is enabled
     */
    public boolean isAutoDetectEnabled() {
        
        boolean autoDetection = false;
       
        // Grab the data from HKEY_CURRENT_USER Registry
        byte[] data = (byte[])WinRegistry.get(WinRegistry.HKEY_CURRENT_USER,
                REGSTR_PATH_AUTOPROXY_DETECT, DEFAULT_CONNECTION_SETTINGS );
        
        // If the key was retrieved Grab the 8th byte
        // Auto Detect boolean is the 4th bit of the 8th byte
        if( data != null && (data[8] & 0x08) != 0 ) {
            autoDetection = true;
        }
        
        return autoDetection;
    }
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    public void getSystemProxy(BrowserProxyInfo bpi) {
    }

	private native ProxyInfo getBrowserProxySettings();

	static class ProxyInfo {
		private int     proxyType;
		private String	proxy;
		private String	proxyBypass;

	    public ProxyInfo(int proxyType, String proxy, String proxyBypass) {
		this.proxyType = proxyType;
		this.proxy = proxy;
		this.proxyBypass = proxyBypass;
	    }

	    public int getProxyType() { return proxyType; }
	    public String getProxy() { return proxy; }
	    public String getProxyBypass() { return proxyBypass; }
	}
}



