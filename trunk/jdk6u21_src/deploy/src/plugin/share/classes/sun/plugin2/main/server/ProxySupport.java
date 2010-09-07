/*
 * @(#)ProxySupport.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.net.URL;
import java.util.List;
import com.sun.deploy.net.proxy.DynamicProxyManager;
import sun.plugin2.message.GetProxyMessage;
import sun.plugin2.message.Pipe;
import sun.plugin2.message.ProxyReplyMessage;

public class ProxySupport {

    static {
        // This will select a particular proxy handler based on the 
        // proxy configuration, e.g. manual or auto proxy script.
        DynamicProxyManager.reset();
    }

    // This is a workaround,used by the MozillaPlugin and
    // MozillaBrowserService, for the complex structure of the proxy
    // handling APIs
    private static final ThreadLocal/*<Plugin>*/ currentPlugin =
        new ThreadLocal();

    protected static void setCurrentPlugin(Plugin plugin) {
        currentPlugin.set(plugin);
    }

    protected static Plugin getCurrentPlugin() {
        return (Plugin) currentPlugin.get();
    }

    public static ProxyReplyMessage getProxyReply(Plugin plugin, GetProxyMessage getProxyMsg) {
        setCurrentPlugin(plugin);
        try {
            return new ProxyReplyMessage(getProxyMsg.getConversation(),
                                         DynamicProxyManager.getProxyList(getProxyMsg.getURL(),
                                                                          getProxyMsg.isSocketURI()));
        } finally {
            setCurrentPlugin(null);
        }
    }

}
        
        
