/*
 * @(#)AssociationDesc.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class AssociationDesc implements XMLable {

    private String _extensions;
    private String _mimeType;
        
    public AssociationDesc(String extensions, String mimeType) {

	_extensions = extensions;
	_mimeType = mimeType;

    }

    public String getExtensions()  { return _extensions; }
    public String getMimeType() { return _mimeType; }
         
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
	ab.add("extensions", _extensions);
	ab.add("mime-type", _mimeType);
	XMLNodeBuilder nb = new XMLNodeBuilder("association",
                                ab.getAttributeList());
	return nb.getNode();
    }
}




