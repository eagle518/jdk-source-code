/*
 * @(#)CookieUnavailableException.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

/**
 * CookieUnavailableException is thrown when cookie
 * service is unavailable for the time being -
 * especially when legacy-lifecycle model is used.
 */
public class CookieUnavailableException extends Exception
{
    /**
     * Create a service unavailable exception object
     */
    public CookieUnavailableException()
    {
    }    

    /**
     * Create a service unavailable exception object
     */
    public CookieUnavailableException(String msg)
    {
	super(msg);
    }    

    /**
     * Create a service unavailable exception object
     */
    public CookieUnavailableException(String msg, Throwable e)
    {
	super(msg);
    }    
}
