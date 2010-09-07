/*
 * @(#)LaunchDescException.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;

/** Root exception for all exceptions that releates
 *  to handling a specific LaunchDesc/JNLP file
 *
 *  It's main thing is to set the category for this
 *  kind of exception.
 */

public class LaunchDescException extends JNLPException {
    private String _message;
    private boolean _isSignedLaunchDesc;
    
    /** Creates an exception */
    public LaunchDescException() {
	this(null);
    }
    
    /** Creates an exception - that wraps another exception */
    public LaunchDescException(Exception e) {
	this(null, e);
    }
    
    /** Sets if the LaunchDesc is signed. This will show a different error message */
    public void setIsSignedLaunchDesc() { _isSignedLaunchDesc = true; }
    public boolean isSignedLaunchDesc() { return _isSignedLaunchDesc;  }
    
    /** Creates an exception - that wraps another exception */
    public LaunchDescException(LaunchDesc ld, Exception e) {
	super(ResourceManager.getString("launch.error.category.launchdesc"), ld, e);
    }
    
    /** Creates an exception with a given message. This message is provided for
     *  convinience when moving to the new Error Handling Architecture. It
     *  should eventually be removed, but we will see how it goes.*/
    public LaunchDescException(LaunchDesc ld, String message, Exception e) {
	this(ld, e);
	_message = message;
    }
    
    /** Returns the message */
    public String getRealMessage() { return _message; }
}


