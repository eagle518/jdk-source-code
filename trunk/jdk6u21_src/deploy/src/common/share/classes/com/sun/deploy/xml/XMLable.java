/*
 * @(#)XMLable.java	1.10 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.xml;

/** Interface that defines if a class can be converted to
 *  an XML document
 */
public interface XMLable {
    public XMLNode asXML();
}


