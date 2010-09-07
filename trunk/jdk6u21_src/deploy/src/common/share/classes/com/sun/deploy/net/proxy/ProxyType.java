/*
 * @(#)ProxyType.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import com.sun.deploy.config.Config;

public interface ProxyType 
{
    /* Proxy config type
     */
    public static final int UNKNOWN = 	Config.PROX_TYPE_UNKNOWN; 
    public static final int NONE = 	Config.PROX_TYPE_NONE;
    public static final int MANUAL = 	Config.PROX_TYPE_MANUAL;
    public static final int AUTO = 	Config.PROX_TYPE_AUTO;
    public static final int BROWSER = 	Config.PROX_TYPE_BROWSER;
    public static final int SYSTEM = Config.PROX_TYPE_SYSTEM;
}



