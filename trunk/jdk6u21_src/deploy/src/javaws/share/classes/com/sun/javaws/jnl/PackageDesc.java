/*
 * @(#)PackageDesc.java	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.jnl;
import com.sun.deploy.xml.*;

/**
 * A resource type object to store information about a package element
 */
public class PackageDesc implements ResourceType {
    private String  _packageName;
    private String  _part;
    private boolean _isRecursive;
    private boolean _isExact;
    
    public PackageDesc(String packageName, String part, boolean isRecursive) {
	if (packageName.endsWith(".*")) {
	    // Remove '*'
	    _packageName = packageName.substring(0, packageName.length() - 1);
	    _isExact = false;
	} else {
	    _isExact = true;
	    _packageName = packageName;
	}
        _part = part;
        _isRecursive = isRecursive;
    }
    
    // Accessors
    String getPackageName() { return _packageName; }
    String getPart() { return _part; }
    boolean isRecursive() { return _isRecursive; }
    
    /** Checks if a resource matches this declaration
     */
    boolean match(String name) {
	if (_isExact) {
	    // Exact match. _isRecursive is ignored
	    return _packageName.equals(name);
	} else if (_isRecursive) {
	    return name.startsWith(_packageName);
	} else  {
	    // Check exact package. Get package of resource
	    int idx = name.lastIndexOf('.');
	    if (idx != -1) name = name.substring(0, idx + 1); // Include dot
	    return name.equals(_packageName);
	}
    }
    
    /** Visitor dispatch */
    public void visit(ResourceVisitor rv) {
	rv.visitPackageDesc(this);
    }
    
    /** Converts to XML */
    public XMLNode asXML() {
	XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("name", getPackageName());
	ab.add("part", getPart());
	ab.add("recursive", isRecursive());
	return new XMLNode("package", ab.getAttributeList());
    }
}

