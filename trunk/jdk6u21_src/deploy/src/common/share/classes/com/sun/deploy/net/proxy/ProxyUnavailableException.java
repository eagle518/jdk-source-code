/*
 * @(#)ProxyUnavailableException.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

/**
 * ProxyUnavailableException is thrown when proxy service
 * browser service is unavailable for the time being -
 * especially when legacy-lifecycle model is used.
 */
public class ProxyUnavailableException extends Exception
{
    /**
     * Create a service unavailable exception object
     */
    public ProxyUnavailableException()
    {
    }    

    /**
     * Create a service unavailable exception object
     */
    public ProxyUnavailableException(String msg)
    {
	super(msg);
    }    

}
