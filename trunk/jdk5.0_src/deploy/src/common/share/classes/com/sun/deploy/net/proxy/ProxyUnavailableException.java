/*
 * @(#)ProxyUnavailableException.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    /**
     * Create a service unavailable exception object
     */
    public ProxyUnavailableException(String msg, Throwable e)
    {
	super(msg, e);
    }    
}
