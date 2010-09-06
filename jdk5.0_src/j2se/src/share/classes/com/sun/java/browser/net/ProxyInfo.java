/*
 * @(#)ProxyInfo.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

