/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

/**
 *  The document title. See the  TITLE element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTitleElement extends HTMLElement 
		    implements org.w3c.dom.html.HTMLTitleElement {

    public HTMLTitleElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The specified title as a string. 
     */
    public String getText() {
	return getAttribute(HTMLConstants.ATTR_TEXT);
    }

    public void setText(String text) {
	setAttribute(HTMLConstants.ATTR_TEXT, text);
    }

}

