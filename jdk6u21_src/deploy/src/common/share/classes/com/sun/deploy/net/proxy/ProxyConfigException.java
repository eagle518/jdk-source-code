/*
 * @(#)ProxyConfigException.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

public final class ProxyConfigException extends Exception
{

    private String _msg = null;
    private Throwable _cause = null;

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
	_msg = msg;
    }    

    /**
     * Create a proxy config exception object
     */
    public ProxyConfigException(String msg, Throwable e)
    {
        super(msg);
	_msg = msg;
	_cause = e;
	e.printStackTrace();
    }    

    public String toString() {
        return "ProxyConfigException: " + _msg + " , " + _cause;
    }
}
