/*
 * @(#)CouldNotLoadArgumentException.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class CouldNotLoadArgumentException extends JNLPException {
    private String _argument;
    
    public CouldNotLoadArgumentException(String arg, Exception ioe) {
	super(ResourceManager.getString("launch.error.category.arguments"), ioe);
	_argument = arg;
    }
    
    /** Returns message */
    public String getRealMessage() {
	return ResourceManager.getString("launch.error.couldnotloadarg", _argument);
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** toString implementation */
    public String toString() { return "CouldNotLoadArgumentException[ " + getRealMessage() + "]"; }
}


