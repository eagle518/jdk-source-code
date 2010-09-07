/*
 * @(#)AssociationDesc.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.deploy.cache;

import java.net.URL;

import com.sun.deploy.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class AssociationDesc implements XMLable {

    private String _extensions;
    private String _mimeType;
    private String _description;
    private URL _icon;
        
    public AssociationDesc(String extensions, String mimeType,
			   String description, URL icon) {

	_extensions = extensions;
	_mimeType = mimeType;
	_description = description;
	_icon = icon;
    }

    public String getExtensions()  { return _extensions; }
    public String getMimeType() { return _mimeType; }
    public String getMimeDescription() { return _description; }
    public URL getIconUrl() { return _icon; }
         
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("extensions", _extensions);
	ab.add("mime-type", _mimeType);
	XMLNodeBuilder nb = new XMLNodeBuilder("association",
                                ab.getAttributeList());

	if (_description != null) {
	    nb.add(new XMLNode("description", null, 
		               new XMLNode(_description), null));
	}
	if (_icon != null) {
	    nb.add(new XMLNode("icon", 
				new XMLAttribute("href", _icon.toString()), 
		   		null, null));
	}
	return nb.getNode();
    }

    private XMLNode getIconNode() {
	XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("href", _icon);
	return new XMLNode("icon", ab.getAttributeList(), null, null);
    }
}




