/*
 * @(#)PluginProxyServiceProvider.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * PluginProxyManagerProvider.java
 *
 * Created on September 26, 2001, 4:01 PM
 */

package sun.plugin.net.proxy;

import java.net.URL;
import com.sun.java.browser.net.ProxyServiceProvider;
import com.sun.java.browser.net.ProxyInfo;
/**
 *
 * @author  Zhengyu Gu
 * @version 
 */
public class PluginProxyServiceProvider implements ProxyServiceProvider {

    public ProxyInfo[] getProxyInfo(URL url) {
        
        com.sun.deploy.net.proxy.ProxyInfo info = com.sun.deploy.net.proxy.DynamicProxyManager.getProxyInfo(url);
        
        if(info.isProxyUsed()) {
            ProxyInfo[] proxies = new ProxyInfo[1]; // for now, we only return one proxy setting
            if(info.isSocksUsed()) {
                // Socks proxy
                proxies[0] = new PluginProxyInfo(info.getSocksProxy(), info.getSocksPort(), true);
            } else {
                proxies[0] = new PluginProxyInfo(info.getProxy(), info.getPort(), false);
            }
            return proxies;
        }
        else {
            return null;    // direct connection
        }
    }
}
