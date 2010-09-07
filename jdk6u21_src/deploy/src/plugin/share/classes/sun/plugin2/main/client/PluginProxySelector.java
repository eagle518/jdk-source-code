/*
 * @(#)PluginProxySelector.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.net.URI;
import java.net.URL;
import java.util.List;
import java.util.ArrayList;
import com.sun.deploy.util.Trace;
import com.sun.deploy.net.proxy.*;
import sun.plugin2.applet.Applet2ExecutionContext;
import sun.plugin2.applet.Applet2Manager;


public class PluginProxySelector extends DeployProxySelector
{
    /**
     * Reset proxy selector.
     */
    public static void initialize()
    {
	// Set proxy selector
	java.net.ProxySelector.setDefault(new PluginProxySelector()); 
    }

    /**
     * Selects all the applicable proxies based on the protocol to
     * access the resource with and a server host name to access the
     * resource at.
     *
     * @param	protocol
     *		The protocol of URL that a connection is required to
     *
     * @param	host
     *		The hostname, or literal address that a connection
     *		is required too.
     *
     * @return	A collection. Each element in the
     *		the collection is of type 
     *          {@link java.net.Proxy Proxy};
     *          when no proxy is available, the collection will
     *          contain one element of type
     *          {@link java.net.Proxy Proxy}
     *          that represents a direct connection.
     * @throws  IllegalArgumentException if either argument is null
     */
    public List select(URI uri)
    {
	if (uri == null)
	    throw new IllegalArgumentException();
        
        List proxyList = null;
        
        try {
            String scheme = uri.getScheme();
            boolean isSocketURI = scheme.equalsIgnoreCase("socket") ||
                scheme.equalsIgnoreCase("serversocket");

            // "getURLFromURI" is defined in super class "DeployProxySelector".
            URL url = getURLFromURI(uri, isSocketURI);
        
            Applet2ExecutionContext appExeCtx = Applet2Manager.getCurrentAppletExecutionContext();
            
            proxyList = appExeCtx.getProxyList(url, isSocketURI);

            if (proxyList.size() > 0) {
                Trace.msgNetPrintln("net.proxy.connect", 
                                    new Object[] {url, 
                                                  proxyList.get(0)});
            }
        } catch (Throwable ex) {
            ex.printStackTrace();
        }
        
        return proxyList;
    } 
}

