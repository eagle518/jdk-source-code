/*
 * @(#)ProxyConfigException.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

public final class ProxyConfigException extends Exception
{
    /**
     * Create a proxy config exception object
     */
    public ProxyConfigException()
    {
    }    

    /**
     * Create a proxy config exception object
     */
    public ProxyConfigException(String msg)
    {
	super(msg);
    }    

    /**
     * Create a proxy config exception object
     */
    public ProxyConfigException(String msg, Throwable e)
    {
	super(msg, e);
    }    
}
