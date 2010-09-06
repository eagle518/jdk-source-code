/*
 * @(#)DOMUnsupportedException.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.dom;


public class DOMUnsupportedException extends Exception
{    
    /**
     * Constructs a new DOMUnsupportedException with no detail message.
     */               
    public DOMUnsupportedException()
    {
	this(null, null);
    }

    /**
     * Constructs a new DOMUnsupportedException with the given detail message.
     *
     * @param msg Detail message.
     */               
    public DOMUnsupportedException(String msg)
    {
	this(null, msg);
    }

    /**
     * Constructs a new DOMUnsupportedException with the given exception as a root clause.
     *
     * @param e Exception.
     */               
    public DOMUnsupportedException(Exception e)
    {
	this(e, null);
    }

    /**
     * Constructs a new DOMUnsupportedException with the given exception as a root clause and the given detail message.
     *
     * @param e Exception.
     * @param msg Detail message.
     */               
    public DOMUnsupportedException(Exception e, String msg)
    {
	this.ex = e;
	this.msg = msg;
    }

    /**
     * Returns the detail message of the error or null if there is no detail message.
     */               
    public String getMessage()
    {
	return msg;
    }

    /**
     * Returns the root cause of the error or null if there is none.
     */               
    public Throwable getCause()
    {
	return ex;
    }

    private Throwable ex;
    private String msg;
}           
