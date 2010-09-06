/*
 * @(#)ProxyType.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
}



