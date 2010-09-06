/*
 * @(#)AbstractView.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.dom.views;

import org.w3c.dom.views.DocumentView;


/**
 *  A class that represents an abstract view of a document.
 */
public class AbstractView implements org.w3c.dom.views.AbstractView
{
    private DocumentView view;

    /**
     * Construct an AbstractView object.
     */
    public AbstractView(DocumentView view)
    {
	this.view = view;
    }

    
    /**
     *  The source <code>DocumentView</code> of which this is an 
     * <code>AbstractView</code> .
     */
    public DocumentView getDocument()
    {
	return view;
    }
}

