/*
 * @(#)PropertyDesc.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    public String getKey() { return _key; }
    public String getValue() { return _value; }
    
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

