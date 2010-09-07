/*
 * @(#)ExtDownloadDesc.java	1.16 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.deploy.Environment;
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
        _isLazy = isLazy;
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



