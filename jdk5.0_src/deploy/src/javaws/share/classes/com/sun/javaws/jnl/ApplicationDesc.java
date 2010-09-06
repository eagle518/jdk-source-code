/*
 * @(#)ApplicationDesc.java	1.13 04/02/02
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.xml.*;

/**
 *   Description of parameters to an application
 */
public class ApplicationDesc implements XMLable {
    private String _mainClass;
    private String[] _arguments;
    
    public ApplicationDesc(String mainClass,
                           String[] arguments) {
        _mainClass = mainClass;
        _arguments = arguments;
    }
    
    /** Get mainclass for an application. If null, the mainclass
     * should be looked up in a manifest.
     */
    public String getMainClass() { return _mainClass; }
    
    /** Returns a set of parameters for the application */
    public String[] getArguments() { return _arguments; }

    public void setArguments(String[] arguments) {
	_arguments = arguments;
    }

        
    /** Converts to XML with or without arguments */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("main-class", _mainClass);
        XMLNodeBuilder nb = new XMLNodeBuilder("application-desc", ab.getAttributeList());
        if (_arguments != null) {
            for(int i = 0; i < _arguments.length; i++) {
                nb.add(new XMLNode("argument", null, new XMLNode(_arguments[i]), null));
            }
        }
        return nb.getNode();
    }
}


