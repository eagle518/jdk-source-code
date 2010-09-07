/*
 * %W% %E%
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;

/**
 * Exception thrown if a parse error occured when interpreting
 * the launch descriptor
 */

public class JNLParseException extends LaunchDescException {
    private String _msg;
    private int _line;
    private String _launchDescSource; // Reference to JNLP file contents
    
    public JNLParseException(String source, Exception exception, String msg, int line) {
	super(exception);
	_msg = msg;
	_line = line;
	_launchDescSource = source;
    }
    
    public int getLine() { return _line; }
    
    public String getRealMessage() {
	if (!isSignedLaunchDesc()) {
	    return ResourceManager.getString("launch.error.parse", _line);
	} else {
	    return ResourceManager.getString("launch.error.parse-signedjnlp", _line);
	}
    };
    
    /** Overwrite this to return the source */
    public String getLaunchDescSource() {
	return _launchDescSource;
	
    }
    
    public String toString() {
	return "JNLParseException[ " + getMessage() + "]"; };
}


