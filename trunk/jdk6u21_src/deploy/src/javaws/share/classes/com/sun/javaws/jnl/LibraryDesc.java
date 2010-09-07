/*
 * @(#)LibraryDesc.java	1.16 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package com.sun.javaws.jnl;

import com.sun.deploy.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class LibraryDesc implements XMLable {

    String _progressClass;

    public LibraryDesc(String progressClass) { 
        _progressClass = progressClass;
    }

    public String getProgressClass() {
        return _progressClass;
    }

    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("progress-class", _progressClass);
        XMLNodeBuilder nb = new XMLNodeBuilder("component-desc", 
                                ab.getAttributeList());
        return nb.getNode();
    }

}




