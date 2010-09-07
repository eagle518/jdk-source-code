/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

/**
 *  Paragraphs. See the  P element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLParagraphElement extends HTMLElement
					implements org.w3c.dom.html.HTMLParagraphElement {

    public HTMLParagraphElement(DOMObject obj, 
				org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Horizontal text alignment. See the  align attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

}

