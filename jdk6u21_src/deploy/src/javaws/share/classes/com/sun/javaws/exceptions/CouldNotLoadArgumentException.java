/*
 * @(#)CouldNotLoadArgumentException.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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


