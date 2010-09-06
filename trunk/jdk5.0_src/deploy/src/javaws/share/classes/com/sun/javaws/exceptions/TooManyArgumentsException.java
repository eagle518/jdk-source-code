/*
 * @(#)TooManyArgumentsException.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class TooManyArgumentsException extends JNLPException {
    private String[] _arguments;
    
    public TooManyArgumentsException(String[] args) {
	super(ResourceManager.getString("launch.error.category.arguments"));
	_arguments = args;
    }
    
    /** Returns message */
    public String getRealMessage() {
	StringBuffer sb = new StringBuffer("{");
	for(int i = 0; i < _arguments.length -1; i ++) {
	    sb.append(_arguments[i]);
	    sb.append(", ");
	}
	sb.append(_arguments[_arguments.length -1]);
	sb.append(" }");
	
	return ResourceManager.getString("launch.error.toomanyargs", sb.toString());
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** toString implementation */
    public String toString() { return "TooManyArgumentsException[ " + getRealMessage() + "]"; }
}


