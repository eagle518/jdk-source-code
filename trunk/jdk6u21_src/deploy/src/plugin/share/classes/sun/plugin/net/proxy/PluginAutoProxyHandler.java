/*
 * @(#)PluginAutoProxyHandler.java	1.49 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import java.io.IOException;
import java.net.URL;
import netscape.javascript.JSObject;
import sun.applet.AppletPanel;
import sun.plugin.viewer.AppletPanelCache;
import sun.plugin.viewer.context.PluginAppletContext;
import sun.plugin.services.BrowserService;
import com.sun.deploy.net.proxy.AbstractAutoProxyHandler;
import com.sun.deploy.net.proxy.ProxyConfigException;
import com.sun.deploy.net.proxy.ProxyInfo;
import com.sun.deploy.util.Trace;


/**
 * Proxy handler for auto proxy configuration.
 */
public final class PluginAutoProxyHandler extends AbstractAutoProxyHandler
{
    /**
     * Return true if auto proxy is handled by IE.
     */
    protected boolean isIExplorer()
    {
	BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();

	return service.isIExplorer();
    }

   
    /**
     * Returns proxy info for a given URL
     *
     * @param u URL
     * @return proxy info for a given URL
     */
    public ProxyInfo[] getProxyInfo(URL u)
		     throws com.sun.deploy.net.proxy.ProxyUnavailableException
    {
	// Auto proxy service in is emulated through JavaScript URL by 
	// using one of the plugin instances. Thus, there MUST be at 
	// least one plugin instance exists. Otherwise, accessing 
	// the proxy service without any plugin instance will likely 
	// to confuse the browser and fail. Thus, it is VERY important 
	// to check if there is at least one applet instance before 
	// calling back to the browser.
	//
	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Proxy service NOT available
	    throw new com.sun.deploy.net.proxy.ProxyUnavailableException("Proxy service unavailable");
	}

	// To determine the proxy info for a given URL,
	// we use JSObject from one of the applets to
	// determine the result.
	//

	Object[] appletPanels = AppletPanelCache.getAppletPanels();
	    
	AppletPanel p = (AppletPanel) appletPanels[0];
	PluginAppletContext pac = (PluginAppletContext) p.getAppletContext();
	String result = null;
		    
	try
	{
	    JSObject win = pac.getJSObject();

	    StringBuffer buffer = new StringBuffer();
	    buffer.append(autoProxyScript);
	    buffer.append("FindProxyForURL('");
	    buffer.append(u);
	    buffer.append("','");
	    buffer.append(u.getHost());
	    buffer.append("');");

	    result = (String) win.eval(buffer.toString());

	    return extractAutoProxySetting(result);
	}
	catch (Throwable e)
	{
	    Trace.msgNetPrintln("net.proxy.auto.result.error");

	    return new ProxyInfo[] {new ProxyInfo(null)};
	}
    }
}



