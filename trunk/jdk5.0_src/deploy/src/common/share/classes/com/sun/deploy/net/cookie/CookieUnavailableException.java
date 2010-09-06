/*
 * @(#)CookieUnavailableException.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
	super(msg, e);
    }    
}
