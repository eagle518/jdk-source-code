/*
 * @(#)ProxyService.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.net;

import java.net.URL;
import java.io.IOException;

/**
 *
 * @author  Zhengyu Gu
 * @version 1.0
 */
public class ProxyService extends Object {
    private static ProxyServiceProvider provider = null;
    

    public static void setProvider(ProxyServiceProvider p) 
    throws IOException {
        if(null == provider)
            provider = p;
        else
            throw new IOException("Proxy service provider has already been set.");
    }

    
    /**
     *  <p>The function returns proxy information of the specified URL.</p>
     *  @param url URL
     *  @return returns proxy information. If there is not proxy, returns null
     *  @since 1.4
     */
    public static ProxyInfo[] getProxyInfo(URL url) 
    throws IOException {
        if(null == provider)
            throw new IOException("Proxy service provider is not yet set");
        
        return provider.getProxyInfo(url);
    }
}
