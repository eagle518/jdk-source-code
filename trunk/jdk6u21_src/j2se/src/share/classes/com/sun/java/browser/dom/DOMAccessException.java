/*
 * @(#)DOMAccessException.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.dom;

public class DOMAccessException extends Exception
{    
    /**
     * Constructs a new DOMAccessException with no detail message.
     */               
    public DOMAccessException()
    {
	this(null, null);
    }


    /**
     * Constructs a new DOMAccessException with the given detail message.
     *
     * @param msg Detail message.
     */               
    public DOMAccessException(String msg)
    {
	this(null, msg);
    }

    /**
     * Constructs a new DOMAccessException with the given exception as a root clause.
     *
     * @param e Exception.
     */               
    public DOMAccessException(Exception e)
    {
	this(e, null);
    }

    /**
     * Constructs a new DOMAccessException with the given exception as a root clause and the given detail message.
     *
     * @param e Exception.
     * @param msg Detail message.
     */               
    public DOMAccessException(Exception e, String msg)
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
