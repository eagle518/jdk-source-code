/*
 * @(#)ProxyInfo.java	1.17 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

/**
 * <p> ProxyInfo is a class that encapsulates a proxy hostname and port.
 * </p>
 */
public final class ProxyInfo  {
    
    /** 
     * <p> Proxy host. </p>
     */
    private String proxy = null;
    
    /** 
     * <p> Proxy port. </p>
     */
    private int port = -1;

    /** 
     * <p> SOCKS proxy host. </p>
     */
    private String socksProxy = null;

    /** 
     * <p> SOCKS proxy port. </p>
     */
    private int socksPort = -1;


    /** 
     * <p> Construct ProxyInfo object.
     * </p>
     *
     * @param pinfo Proxy info string.
     */
    public ProxyInfo(String pinfo)  {
	this(pinfo, null);
    }


    /** 
     * <p> Construct ProxyInfo object.
     * </p>
     *
     * @param pinfo Proxy info string.
     * @param spinfo SOCKS proxy info string.
     */
    public ProxyInfo(String pinfo, String spinfo)  {
	if (pinfo != null)
	{
	    // Extract proxy setting from proxy URL, e.g. http://webcache-cup:8080
	    int ix = pinfo.indexOf("//");
	    if (ix >= 0)
		pinfo = pinfo.substring(ix + 2);

            // Parse the proxy setting from a string "hostname:port".
	    ix = pinfo.lastIndexOf(':');
	    if (ix >= 0)  {
		proxy = pinfo.substring(0, ix);
		try  {
		    port = Integer.parseInt(pinfo.substring(ix + 1).trim());
		} catch (Exception e)  {
		}
	    }
	    else  {
		if (!pinfo.equals(""))
		    proxy = pinfo;
	    }
	}

	if (spinfo != null)
	{
            // Parse the SOCKS proxy setting from a string "hostname:port".
	    int ix = spinfo.lastIndexOf(':');
	    if (ix >= 0)  {
		socksProxy = spinfo.substring(0, ix);
		try  {
		    socksPort = Integer.parseInt(spinfo.substring(ix + 1).trim());
		} catch (Exception e)  {
		}
	    }
	    else  {
		if (!spinfo.equals(""))
		    socksProxy = spinfo;
	    }
	}
    }


    /** 
     * <p> Construct ProxyInfo object.
     * </p>
     *
     * @param proxy Proxy host.
     * @param port Proxy port.
     */
    public ProxyInfo(String proxy, int port)  {
	this(proxy, port, null, -1);
    }

    /** 
     * <p> Construct ProxyInfo object.
     * </p>
     *
     * @param proxy Proxy host.
     * @param port Proxy port.
     * @param socksProxy SOCKS proxy host.
     * @param socksPort SOCKS proxy port.
     */
    public ProxyInfo(String proxy, int port, String socksProxy, int socksPort)  {
        this.proxy = proxy;
        this.port = port;
	this.socksProxy = socksProxy;
	this.socksPort = socksPort;
    }

    /**
     * <p> Returns proxy host. </p>
     */
    public String getProxy()  {
        return proxy;
    }

    /**
     * <p> Returns proxy port. </p>
     */
    public int getPort()  {
        return port;
    }

    /**
     * <p> Returns SOCKS proxy host. </p>
     */
    public String getSocksProxy() {
	return socksProxy;
    }

    /**
     * <p> Returns SOCKS proxy port. </p>
     */
    public int getSocksPort() {
	return socksPort;
    }

    /** 
     * <p> Returns true if normal proxy or SOCKS proxy is used. </p>
     */
    public boolean isProxyUsed() {
	return (proxy != null || socksProxy != null);
    }

    /**
     * <p> Returns true if SOCKS is used. </p>
     */
    public boolean isSocksUsed() {
	return (socksProxy != null);
    }
    
    /**
     * <p> Returns proxy host and port </p>
     */
     public String toString() {
	if (proxy != null)
	{
	    return proxy + ":" + port;
	}
	else if (socksProxy != null)
	{
	    return socksProxy + ":" + socksPort;
	}
	else
	{
	    return "DIRECT";
	}
    }
}




