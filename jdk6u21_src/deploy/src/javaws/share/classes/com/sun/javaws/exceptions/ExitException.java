/*
 * @(#)ExitException.java	1.8 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;

/**
 * Exception thrown when for any reason Java Web Start cannot run
 */

public class ExitException extends Exception {
    private int _reason;
    private Throwable _throwable;

    public static final int OK = 0;
    public static final int REBOOT = 1;
    public static final int JRE_MISMATCH = 2;
    public static final int LAUNCH_ERROR = 3;

    // user denied cert, or invalid cert and UI shown
    public static final int LAUNCH_ABORT_SILENT = 4;
    
    // using SingleInstanceService and singleton instance is already running
    public static final int LAUNCH_SINGLETON = 5;

    public ExitException(Throwable throwable, int reason) {
        _throwable = throwable;
        _reason = reason;
    }
    
    public Throwable getException() { return _throwable; }

    public int getReason() { return _reason; }

    // @return true if this exception represents an error, else false
    public boolean isErrorException() { return _reason!=OK && _reason!=LAUNCH_SINGLETON; }

    // @return true if this exception shall not raise a UI, else false
    public boolean isSilentException() { return _reason==OK || _reason>=LAUNCH_ABORT_SILENT; }

    public String getMessage() {
	if (_throwable != null && _throwable.getMessage() != null) {
	    return _throwable.getMessage();
	}
	
	return this.toString();
    }
    
    public String toString() {
        String str = "ExitException[ " + getReason() + "]"; 
        if (_throwable != null) { 
            str += _throwable.toString();
        }
        return str;
    }
}


