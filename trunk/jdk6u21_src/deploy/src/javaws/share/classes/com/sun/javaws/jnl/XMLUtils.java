/*
 * @(#)XMLUtils.java	1.21 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import java.net.URL;
import java.net.MalformedURLException;
import com.sun.javaws.exceptions.BadFieldException;
import com.sun.javaws.exceptions.MissingFieldException;
import com.sun.deploy.xml.*;
import com.sun.deploy.util.Trace;

import com.sun.deploy.util.*;

/** Contains handy methods for looking up information
 *  stored in XMLNodes.
 */
public class XMLUtils {
        
    /** Returns the value of an integer attribute */
    static public int getIntAttribute(String source, XMLNode root, String path, String name, int defaultvalue)
        throws BadFieldException {
        String value = getAttribute(root, path, name);
        if (value == null) return defaultvalue;
        try {
	    return Integer.parseInt(value);
        } catch(NumberFormatException nfe) {
	    throw new BadFieldException(source, getPathString(root) + path + name, value);
        }
    }
    
    /** Returns the value of an integer attribute */
    static public int getRequiredIntAttribute(String source, XMLNode root, String path, String name)
        throws BadFieldException, MissingFieldException {
        String value = getAttribute(root, path, name);
        if (value == null) throw new MissingFieldException(source, getPathString(root) + path + name);
        try {
	    return Integer.parseInt(value);
        } catch(NumberFormatException nfe) {
	    throw new BadFieldException(source, getPathString(root) + path + name, value);
        }
    }
    
    /** Returns the value of a given attribute, or null if not set */
    static public String getAttribute(XMLNode root, String path, String name) {
        return getAttribute(root, path, name, null);
    }
    
    /** Returns the value of a given attribute */
    static public String getRequiredAttributeEmptyOK(String source,
	XMLNode root, String path, String name) throws MissingFieldException {
	String value = null;
        XMLNode elem = findElementPath(root, path);
        if (elem != null) {
            value = elem.getAttribute(name);
	}
        if  (value == null) {
            throw new MissingFieldException(source,
					    getPathString(root)+ path + name);
	}
	return value;
    }

    /** Returns the value of a given attribute, or null if not set */
    static public String getRequiredAttribute(String source, XMLNode root, String path, String name) throws MissingFieldException {
        String s = getAttribute(root, path, name, null);
        if (s == null) throw new MissingFieldException(source, getPathString(root)+ path + name);
        s = s.trim();
        return (s.length() == 0) ? null : s;
    }
    
    /** Returns the value of a given attribute, or the default value 'def' if not set */
    static public String getAttribute(XMLNode root, String path, String name, String def) {
        XMLNode elem = findElementPath(root, path);
        if (elem == null) return def;
        String value = elem.getAttribute(name);
        return (value == null || value.length() == 0) ? def : value;
    }
    
    /** Expands a URL into an absolute URL from a relative URL */
    static public URL getAttributeURL(String source, URL base, XMLNode root, String path, String name) throws BadFieldException {
        String value = getAttribute(root, path, name);
        if (value == null) return null;
        try {
	    if (value.startsWith("jar:")) {
		int bang = value.indexOf("!/");
		if (bang > 0) {
		    String entry = value.substring(bang);
		    String urlString = value.substring(4, bang);
		    URL url = (base == null) ? 
			new URL(urlString) : new URL(base, urlString);
		    return new URL("jar:" + url.toString() + entry);
		}
	    }
	    return (base == null) ? new URL(value) : new URL(base, value);
        } catch(MalformedURLException mue) {
	    if (mue.getMessage().indexOf("https") != -1) {
		throw new BadFieldException(source, "<jnlp>", "https");
	    } 	    
	    throw new BadFieldException(source, getPathString(root) + path + name, value);
        }
    }
    
    /** Returns the value of an attribute as a URL or null if not set */
    static public URL getAttributeURL(String source, XMLNode root, String path, String name) throws BadFieldException {
        return getAttributeURL(source, null, root, path, name);
    }
    
    static public URL getRequiredURL(String source, URL base, XMLNode root, String path, String name) throws BadFieldException, MissingFieldException {
        URL url = getAttributeURL(source, base, root, path, name);
        if (url == null) throw new MissingFieldException(source, getPathString(root) + path + name);
        return url;
    }
    
    /** Returns the value of an attribute as a URL. Throws a MissingFieldException if the
     *  attribute is not defined
     */
    static public URL getRequiredURL(String source, XMLNode root, String path, String name) throws BadFieldException, MissingFieldException {
        return getRequiredURL(source, null, root, path, name);
    }
    
    /** Returns true if the path exists in the document, otherwise false */
    static public boolean isElementPath(XMLNode root, String path) {
        return findElementPath(root, path) != null;
    }
    
    static public URL getElementURL(String source, XMLNode root, String path) throws BadFieldException {
        String value = getElementContents(root, path);
        try {
	    return new URL(value);
        } catch(MalformedURLException mue) {
	    throw new BadFieldException(source, getPathString(root) + path, value);
        }
    }
    
    /** Returns a string describing the current location in the DOM */
    static public String getPathString(XMLNode e) {
        return (e == null || !(e.isElement())) ? "" : getPathString(e.getParent()) + "<" + e.getName() + ">";
    }
    
    /** Returns the contents of an element with the given path and an attribute matching a specific value. Returns
     *  NULL if not found
     */
    static public String getElementContentsWithAttribute(XMLNode root, String path, String attr, String val, String defaultvalue)
        throws BadFieldException, MissingFieldException {
        XMLNode e = getElementWithAttribute(root, path, attr, val);
        if (e == null) return defaultvalue;
        return getElementContents(e, "", defaultvalue);
    }
    
    static public URL getAttributeURLWithAttribute(String source, XMLNode root, String path, String attrcond, String val,
						   String name, URL defaultvalue)
        throws BadFieldException, MissingFieldException {
        XMLNode e = getElementWithAttribute(root, path, attrcond, val);
        if (e == null) return defaultvalue;
        URL url = getAttributeURL(source, e, "", name);
        if (url == null) return defaultvalue;
        return url;
    }
    
    
    /** Returns an element with the given path and an attribute matching a specific value. Returns
     *  NULL if not found
     */
    static public XMLNode getElementWithAttribute(XMLNode root, String path, final String attr, final String val)
        throws BadFieldException, MissingFieldException {
        final XMLNode[] result = { null };
        visitElements(root, path, new ElementVisitor() {
		    public void visitElement(XMLNode  e) throws BadFieldException, MissingFieldException {
			if (result[0] == null && e.getAttribute(attr).equals(val)) {
			    result[0] = e;
			}
		    }
		});
        return result[0];
    }
    
    /** Like getElementContents(...) but with a defaultValue of null */
    static public String getElementContents(XMLNode root, String path) {
        return getElementContents(root, path, null);
    }
    
    /** Returns the value of the last element tag in the path, e.g.,  <..><tag>value</tag>. The DOM is assumes
     *  to be normalized. If no value is found, the defaultvalue is returned
     */
    static public String getElementContents(XMLNode root, String path, String defaultvalue) {
        XMLNode e = findElementPath(root, path);
	if (e == null) return defaultvalue;
        XMLNode n = e.getNested();
        if (n != null && !n.isElement()) return n.getName();
	return defaultvalue;
    }
    
    /** Parses a path string of the form <tag1><tag2><tag3> and returns the specific Element
     *  node for that tag, or null if it does not exist. If multiple elements exists with same
     *  path the first is returned
     */
    static public XMLNode findElementPath(XMLNode elem, String path) {
	// End condition. Root null -> path does not exist
	if (elem == null) return null;
	// End condition. String empty, return current root
	if (path == null || path.length() == 0) return elem;
	
	// Strip of first tag
	int idx = path.indexOf('>');
	if (!(path.charAt(0) == '<')) { 
	    throw new IllegalArgumentException("bad path. Missing begin tag");
	}
	if (idx == -1) {
	    throw new IllegalArgumentException("bad path. Missing end tag");
	}
	String head = path.substring(1, idx);
	String tail = path.substring(idx + 1);
	return findElementPath(findChildElement(elem, head), tail);
    }
    
    /** Returns an child element with the current tag name or null. */
    static public XMLNode findChildElement(XMLNode elem, String tag) {
	XMLNode n = elem.getNested();
	while(n != null) {
	    if (n.isElement() && n.getName().equals(tag)) return n;
	    n = n.getNext();
	}
	return null;
    }
    
    /** Iterator class */
    public abstract static class ElementVisitor {
	abstract public void visitElement(XMLNode e) throws BadFieldException, MissingFieldException;
    }
    
    /** Visits all elements which matches the <path>. The iteration is only
     *  done on the last elment in the path.
     */
    static public void visitElements(XMLNode root, String path, ElementVisitor ev)
	throws BadFieldException, MissingFieldException{
	// Get last element in path
	int idx = path.lastIndexOf('<');
	if (idx == -1) {
	    throw new IllegalArgumentException(
		"bad path. Must contain atleast one tag");
	}
	if (path.length() == 0 || path.charAt(path.length() - 1) != '>') {
	    throw new IllegalArgumentException("bad path. Must end with a >");
	}
	String head = path.substring(0, idx);
	String tag  = path.substring(idx + 1, path.length() - 1);
	
	XMLNode elem = findElementPath(root, head);
	if (elem == null) return;
	
	// Iterate through all child nodes
	XMLNode n = elem.getNested();
	while(n != null) {
	    if (n.isElement() && n.getName().equals(tag)) {
		ev.visitElement(n);
	    }
	    n = n.getNext();
	}
    }
    
    static public void visitChildrenElements(XMLNode elem, ElementVisitor ev)
	throws BadFieldException, MissingFieldException {
	// Iterate through all child nodes
	XMLNode n = elem.getNested();
	while(n != null) {
	    if (n.isElement()) ev.visitElement(n);
	    n = n.getNext();
	}
    }
}

