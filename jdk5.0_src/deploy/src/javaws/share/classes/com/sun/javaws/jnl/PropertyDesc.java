/*
 * @(#)PropertyDesc.java	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.xml.*;
/**
 * A object to store information about property elements
 */
public class PropertyDesc implements ResourceType {
    private String _key;
    private String _value;
    
    public PropertyDesc(String key, String value) {
	_key = key;
	_value = value;
    }
    
    // Accessors
    String getKey() { return _key; }
    String getValue() { return _value; }
    
    /** Visitor dispatch */
    public void visit(ResourceVisitor rv) {
	rv.visitPropertyDesc(this);
    }
    
    /** Converts to XML */
    public XMLNode asXML() {
	return new XMLNode("property",
			   new XMLAttribute("name", getKey(),
			   new XMLAttribute("value", getValue())), null, null);
    }
}

