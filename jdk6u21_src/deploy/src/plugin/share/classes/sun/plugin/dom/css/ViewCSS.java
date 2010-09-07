/*
 * @(#)ViewCSS.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.dom.css;

import org.w3c.dom.views.DocumentView;
import org.w3c.dom.views.AbstractView;
import sun.plugin.dom.views.*;


/**
 *  This interface represents a CSS view. The <code>getComputedStyle</code> 
 * method provides a read only access to the computed values of an element. 
 * <p> The expectation is that an instance of the <code>ViewCSS</code> 
 * interface can be obtained by using binding-specific casting methods on an 
 * instance of the <code>AbstractView</code> interface. 
 * <p> Since a computed style is related to an <code>Element</code> node, if 
 * this element is removed from the document, the associated 
 * <code>CSSStyleDeclaration</code> and <code>CSSValue</code> related to 
 * this declaration are no longer valid. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class ViewCSS extends sun.plugin.dom.views.AbstractView
		     implements org.w3c.dom.css.ViewCSS	    
{
    /**
     * Construct an AbstractView object.
     */
    public ViewCSS(org.w3c.dom.views.DocumentView view)
    {
	super(view);
    }

    /**
     *  This method is used to get the computed style as it is defined in . 
     * @param elt The element whose style is to be computed. This parameter 
     *   cannot be null. 
     * @param pseudoElt The pseudo-element or <code>null</code> if none. 
     * @return  The computed style. The <code>CSSStyleDeclaration</code> is 
     *   read-only and contains only absolute values. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getComputedStyle(org.w3c.dom.Element elt, 
                                                String pseudoElt)
    {
	org.w3c.dom.css.DocumentCSS documentCSS = (org.w3c.dom.css.DocumentCSS) getDocument();

	return documentCSS.getOverrideStyle(elt, pseudoElt);
    }
}

