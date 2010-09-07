/*
 * @(#)ApplicationDesc.java	1.17 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.xml.*;

/**
 *   Description of parameters to an application
 */
public class ApplicationDesc implements XMLable {
    private String _mainClass;
    private String _progressClass;
    private String[] _arguments;
    
    public ApplicationDesc(String mainClass, String progressClass,
                           String[] arguments) {
        _mainClass = mainClass;
        _progressClass = progressClass;
        _arguments = arguments;
    }
    
    /** Get mainclass for an application. If null, the mainclass
     * should be looked up in a manifest.
     */
    public String getMainClass() { return _mainClass; }
    public String getProgressClass() { return _progressClass; }
    
    /** Returns a set of parameters for the application */
    public String[] getArguments() { return _arguments; }

    public void setArguments(String[] arguments) {
	_arguments = arguments;
    }

        
    /** Converts to XML with or without arguments */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("main-class", _mainClass);
        ab.add("progress-class", _progressClass);
        XMLNodeBuilder nb = new XMLNodeBuilder("application-desc", ab.getAttributeList());
        if (_arguments != null) {
            for(int i = 0; i < _arguments.length; i++) {
                nb.add(new XMLNode("argument", null, new XMLNode(_arguments[i]), null));
            }
        }
        return nb.getNode();
    }
}


