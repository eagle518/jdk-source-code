/*
 * @(#)ExtDownloadDesc.java	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.Globals;
import com.sun.deploy.xml.*;

/**
 *   Encapsulates information in the <ext-download .../> element
 */
public class ExtDownloadDesc implements XMLable {
    private String _extensionPart;    // Name of part in extension JNLP file
    private String _part;    // Name of part in this JNLP file
    private boolean _isLazy; // eager/lazy download
    
    public ExtDownloadDesc (String extPart, String part, boolean isLazy) {
        _extensionPart = extPart;
	_part = part;
        _isLazy = isLazy && !Globals.isImportMode();  // no lazy in inport mode
    }
    
    public String getExtensionPart() { return _extensionPart; }
    public String getPart() { return _part; }
    public boolean isLazy()  { return _isLazy; }
    
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("ext-part", _extensionPart);
        ab.add("part", _part);
        ab.add("download", _isLazy ? "lazy" : "eager");
        return new XMLNode("ext-download", ab.getAttributeList());
    }
}



