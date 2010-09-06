/*
 * @(#)ProxyServiceProvider.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.net;

import java.net.URL;

/**
 *
 * @author  Zhengyu Gu
 * @version 1.0
 */
public interface ProxyServiceProvider {
    public ProxyInfo[] getProxyInfo(URL url);
}

