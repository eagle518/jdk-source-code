/*
 * @(#)XMLAttributeBuilder.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.xml;
import java.util.ArrayList;
import java.net.URL;

/** Utility class for constructing XMLAttribute lists
 */
public class XMLAttributeBuilder {
    private XMLAttribute _root;
    private XMLAttribute _next;
    
    public XMLAttributeBuilder() {
	_root = null;
    }
    
    public void add(XMLAttribute attr) {
	if (attr != null) {
	    if (_next == null) {
		_root = _next = attr;
		attr.setNext(null);
	    } else {
		_next.setNext(attr);
		_next = attr;
		attr.setNext(null);
	    }
	}
    }
    
    /** Add a string attribute to the list. NULL arguments are ignored */
    public void add(String name, String value) {
	if (value != null && value.length() > 0) {
	    add(new XMLAttribute(name, value));
	}
    }
    
    /** Add a URL attribute to the list. NULL arguments are ignored */
    public void add(String name, URL value) {
	if (value != null) {
	    add(new XMLAttribute(name, value.toString()));
	}
    }
    
    /** Add an integer attribute to the list. NULL arguments are ignored */
    public void add(String name, long value) {
	if (value != 0) {
	    add(new XMLAttribute(name, new Long(value).toString()));
	}
    }
    
    /** Add an boolean attribute to the list. NULL arguments are ignored */
    public void add(String name, boolean value) {
	add(new XMLAttribute(name, (value) ? "true" : "false" ));
    }
    
    /** Returns list of all added attributes in the right order*/
    public XMLAttribute getAttributeList() { return _root; }
}


