/*
 * @(#)ProxyServiceProvider.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

