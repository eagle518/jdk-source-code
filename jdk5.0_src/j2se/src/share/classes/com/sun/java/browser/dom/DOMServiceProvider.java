/*
 * @(#)DOMServiceProvider.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.dom;

public abstract class DOMServiceProvider
{
    /**
     * An empty constructor is provided. Implementations should 
     * provide a public constructor so that the DOMService
     * can instantiate instances of the implementation class. 
     * Application programmers should not be able to directly
     * construct implementation subclasses of this abstract subclass.
     * The only way an application should be able to obtain a 
     * reference to a DOMServiceProvider implementation
     * instance is by using the appropriate methods of the
     * DOMService.
     */
    public DOMServiceProvider()
    {
    }

    /**
     * Returns true if the DOMService can determine the association
     * between the obj and the underlying DOM in the browser.
     */
    public abstract boolean canHandle(Object obj);

    /**
     * Returns the Document object of the DOM.
     */
    public abstract org.w3c.dom.Document getDocument(Object obj) throws DOMUnsupportedException;

    /**
     * Returns the DOMImplemenation object of the DOM.
     */
    public abstract org.w3c.dom.DOMImplementation getDOMImplementation();
}            
