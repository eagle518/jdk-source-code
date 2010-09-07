/*
 * @(#)InvalidArgumentException.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.deploy.resources.ResourceManager;

public class InvalidArgumentException extends JNLPException {
    private String[] _arguments;
    
    public InvalidArgumentException(String[] args) {
	super(ResourceManager.getString("launch.error.category.arguments"));
	_arguments = args;
    }
    
    /** Returns message */
    public String getRealMessage() {
	StringBuffer sb = new StringBuffer("{");
	for(int i = 0; i < _arguments.length; i ++) {
	    sb.append(_arguments[i]);
	    if (i < _arguments.length - 1) {
	        sb.append(", ");
	    }
	}
	sb.append(" }");
	
	return ResourceManager.getString("launch.error.toomanyargs", sb.toString());
    }
    
    /** Returns the name of the offending field */
    public String getField() { return getMessage(); }
    
    /** toString implementation */
    public String toString() { return "InvalidArgumentException[ " + getRealMessage() + "]"; }
}


