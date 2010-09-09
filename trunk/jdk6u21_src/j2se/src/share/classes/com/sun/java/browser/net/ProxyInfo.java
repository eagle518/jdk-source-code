/*
 * @(#)ProxyInfo.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.net;

/**
 *
 * @author  Zhengyu Gu
 * @version 
 */
public interface ProxyInfo {
    public String   getHost();
    public int      getPort();
    public boolean  isSocks();
}

