/*
 * @(#)XMLable.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.xml;

/** Interface that defines if a class can be converted to
 *  an XML document
 */
public interface XMLable {
    public XMLNode asXML();
}


