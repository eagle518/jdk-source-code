/*
 * @(#)BrowserProxyConfig.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;


/**
 * Interface that encapsulates the browser proxy config
 */
public interface BrowserProxyConfig 
{
    /* 
     * Returns browser proxy info
     */
    BrowserProxyInfo getBrowserProxyInfo();
    
    /**
     * add system proxy info to BrowserProxyInfo
     */
    void getSystemProxy(BrowserProxyInfo bpi);
}



