/*
 * @(#)PluginProxyInfo.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * PluginProxyInfo.java
 *
 * Created on September 26, 2001, 4:13 PM
 */

package sun.plugin.net.proxy;

import com.sun.java.browser.net.ProxyInfo;
/**
 *
 * @author  Zhengyu Gu
 * @version 
 */
public class PluginProxyInfo implements ProxyInfo{

    private String  host;
    private int     port;
    private boolean socks;
    
    /** Creates new PluginProxyInfo */
    public PluginProxyInfo() {
        // default as direct proxy
        host = null;
        port = -1;
        socks = false;
    }
    
    public PluginProxyInfo(String host, int port, boolean isSocks) {
        this.host = host;
        this.port = port;
        this.socks = isSocks;
    }
    
    public String getHost() {
        return this.host;
    }
    
    public int getPort() {
        return this.port;
    }
    
    public boolean isSocks() {
        return this.socks;
    }
}
