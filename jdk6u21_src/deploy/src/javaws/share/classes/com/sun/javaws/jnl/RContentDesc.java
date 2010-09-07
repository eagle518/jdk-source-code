/*
 * @(#)RContentDesc.java	1.6 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;
import java.net.URL;

/**
 *   Tag for a extension library descriptor
 */
public class RContentDesc implements XMLable {

    private URL _href;
    private String _title;
    private String _description;
    private URL _icon;
    private boolean _isApplication;
        
    public RContentDesc(URL href, String title, String description, URL icon) {
        _href = href;
	_title = title;	
        _description = description;
        _icon = icon;
	_isApplication = (href != null && href.toString().endsWith(".jnlp"));
    }

    public URL getHref() { return _href; }
    public URL getIcon() { return _icon; }
    public String getTitle() {
	// fix for 5074087: if title is not specified for related-content,
	// app cannot start on windows
	// we do not set _title here even if it is null, because
	// we want to reflect what is really in the jnlp file in asXML
	if (_title == null) {
	    String fullPath = _href.getPath();
	    return fullPath.substring(fullPath.lastIndexOf('/') + 1);
	}
	return _title; 
    }
    public String getDescription() { return _description; }
    public boolean isApplication() { return _isApplication; }
         
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("href", _href);
        XMLNodeBuilder nb = new XMLNodeBuilder("related-content",
                                ab.getAttributeList());
	if (_title != null) {
            nb.add("title", _title);
	} 
        if (_description != null) {
	    nb.add("description", _description);
	}
	if (_icon != null) {
	    nb.add(new XMLNode("icon", 
				new XMLAttribute("href", _icon.toString())));
	}
	return nb.getNode();
    }
}




