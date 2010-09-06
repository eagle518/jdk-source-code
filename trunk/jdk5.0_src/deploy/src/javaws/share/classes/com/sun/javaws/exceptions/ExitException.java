/*
 * @(#)ExitException.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

/**
 * Exception thrown if a parse error occured when interpreting
 * the launch descriptor
 */

public class ExitException extends Exception {
    private int _reason;
    private Exception _exception;

    public static final int OK = 0;
    public static final int REBOOT = 1;
    public static final int LAUNCH_ERROR = 2;
    
    public ExitException(Exception exception, int reason) {
	_exception = exception;
	_reason = reason;
    }
    
    public Exception getException() { return _exception; }

    public int getReason() { return _reason; }
    
    public String toString() {
	String str = "ExitException[ " + getReason() + "]"; 
	if (_exception != null) { 
	    str += _exception.toString();
	}
	return str;
    }
}


