/*
 * @(#)WIExplorerProxyConfig.java	1.21 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    private static final String REGSTR_VAL_PROXYENABLE = "ProxyEnable";
    private static final String REGSTR_VAL_PROXYSERVER = "ProxyServer";
    private static final String REGSTR_VAL_PROXYOVERRIDE = "ProxyOverride";
    private static final String REGSTR_VAL_AUTOCONFIGURL = "AutoConfigURL";


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

		if(proxyInfo == null || info.getType() == ProxyType.NONE) {	// InternetGetOption API does not handle auto proxy
			int key = WinRegistry.HKEY_CURRENT_USER; 
			// Check if auto config is enabled 
			String autoConfigURL = WinRegistry.getString(key, REGSTR_PATH_INTERNET_SETTINGS, REGSTR_VAL_AUTOCONFIGURL); 

			if (autoConfigURL != null)  { 
				Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL}); 
				info.setType(ProxyType.AUTO); 
				info.setAutoConfigURL(autoConfigURL); 
			}
		}

		Trace.msgNetPrintln("net.proxy.loading.done");		
		return info;
    }

	private native ProxyInfo getBrowserProxySettings();

	static class ProxyInfo {
		private int		proxyType;
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



