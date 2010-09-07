/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Preformatted text. See the  PRE element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLPreElement extends HTMLElement
				  implements org.w3c.dom.html.HTMLPreElement {

    public HTMLPreElement(DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Fixed width for content. See the  width attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public int getWidth() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(int width) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_WIDTH, width);
    }
}

