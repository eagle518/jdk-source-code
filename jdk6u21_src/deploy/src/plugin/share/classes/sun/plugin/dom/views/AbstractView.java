/*
 * @(#)AbstractView.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

