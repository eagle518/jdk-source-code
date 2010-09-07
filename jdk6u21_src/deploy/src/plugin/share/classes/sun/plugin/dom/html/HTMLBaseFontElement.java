/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

public class HTMLBaseFontElement extends HTMLElement
				implements org.w3c.dom.html.HTMLBaseFontElement {

    public HTMLBaseFontElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Font color. See the  color attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getColor() {
	return getAttribute(HTMLConstants.ATTR_COLOR);
    }
        
    public void setColor(String color) {
	setAttribute(HTMLConstants.ATTR_COLOR, color);
    }

    /**
     *  Font face identifier. See the  face attribute definition in HTML 4.0. 
     * This attribute is deprecated in HTML 4.0.
     */
    public String getFace() {
	return getAttribute(HTMLConstants.ATTR_FACE);
    }

    public void setFace(String face) {
	setAttribute(HTMLConstants.ATTR_FACE, face);
    }

    /**
     *  Font size. See the  size attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getSize() {
	return getAttribute(HTMLConstants.ATTR_SIZE);
    }

    public void setSize(String size) {
	setAttribute(HTMLConstants.ATTR_SIZE, size);
    }
}
