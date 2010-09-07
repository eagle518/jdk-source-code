/*
 * @(#)WIExplorerAutoProxyHandler.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import com.sun.deploy.net.proxy.AbstractAutoProxyHandler;
import com.sun.deploy.net.proxy.ProxyConfigException;
import com.sun.deploy.net.proxy.ProxyInfo;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;


/**
 * Proxy handler for auto proxy configuration.
 */
public final class WIExplorerAutoProxyHandler extends AbstractAutoProxyHandler
{
    /**
     * Return true if auto proxy is handled by IE.
     */
    protected boolean isIExplorer()
    {
	return true;
    }


    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
    {
	String result = null;
		    
	try
	{
	    StringBuffer buffer = new StringBuffer();
	    if (Config.isJavaVersionAtLeast14()) {
	        buffer.append(autoProxyScript);
	    } else {
		buffer.append(autoProxyScript.toString());
	    }
	    buffer.append("FindProxyForURL('");
	    buffer.append(u);
	    buffer.append("','");
	    buffer.append(u.getHost());
	    buffer.append("');");

	    result = evalScript(buffer.toString());

	    return extractAutoProxySetting(result);
	}
	catch (Throwable e)
	{
	    Trace.ignored(e);
	    Trace.msgNetPrintln("net.proxy.auto.result.error");
	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
    }


    /**
     * Native method to evaluate JavaScript.
     */
    private native String evalScript(String script);
}
