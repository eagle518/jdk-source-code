/*
 * @(#)XMLNodeBuilder.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.xml;
import java.util.ArrayList;
import java.net.URL;

public class XMLNodeBuilder {
    private XMLNode _root;
    private XMLNode _last;
    
    public XMLNodeBuilder(String name, XMLAttribute attrs) {
	_root = new XMLNode(name, attrs, null, null);
	_last = null;
    }
    
    /** Add a nested entry to this node - order is preserved */
    public void add(XMLNode n) {
	if (n == null) return;
	if (_last == null) {
	    _root.setNested(n);
	    _last = n;
	} else {
	    _last.setNext(n);
	    n.setNext(null);
	    _last = n;
	}
    }
    
    /** Add a nested entry to this node */
    public void add(XMLable node) {
	if (node == null) return;
	add(node.asXML());
    }
    
    /** Add an node with a given name and value */
    public void add(String name, String value) {
	if (value != null) {
	    add(new XMLNode(name, null, new XMLNode(value), null));
	}
    }
    
    public XMLNode getNode() { return _root; }
}




