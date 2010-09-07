/*
 * @(#)JreExecException.java	1.11 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.exceptions;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;

/**
 * Exception thrown if an exec. of a JRE failed
 *
 */
public class JreExecException extends JNLPException {
    private String _version;
    
    public JreExecException(String version, Exception e) {
	super(ResourceManager.getString("launch.error.category.unexpected"), e);
	_version = version;
    }
    
    public String getRealMessage() {
	return ResourceManager.getString("launch.error.failedexec", _version);
    }
        
    public String toString() {
	return "JreExecException[ " + getMessage() + "]"; };
}


