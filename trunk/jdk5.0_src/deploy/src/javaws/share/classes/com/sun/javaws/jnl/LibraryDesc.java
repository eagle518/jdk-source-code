/*
 * @(#)LibraryDesc.java	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class LibraryDesc implements XMLable {

    private String _uniqueId;
        
    public LibraryDesc(String uniqueId) { 
	_uniqueId = uniqueId;
    }

    public String getUniqueId() {
	return _uniqueId;
    }
         
    /** Outputs as XML */
    public XMLNode asXML() {
        return new XMLNode("library-desc", 
			   new XMLAttribute("unique-id", _uniqueId));
    }

}




