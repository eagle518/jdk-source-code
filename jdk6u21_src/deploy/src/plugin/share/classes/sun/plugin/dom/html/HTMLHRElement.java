/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;


/**
 *  Create a horizontal rule. See the  HR element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLHRElement extends HTMLElement 
				 implements org.w3c.dom.html.HTMLHRElement {

    public HTMLHRElement(DOMObject obj, 
			 org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Align the rule on the page. See the  align attribute definition in 
     * HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /**
     *  Indicates to the user agent that there should be no shading in the 
     * rendering of this element. See the  noshade attribute definition in 
     * HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public boolean getNoShade() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_NO_SHADE);
    }

    public void setNoShade(boolean noShade) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_NO_SHADE, noShade);
    }

    /**
     *  The height of the rule. See the  size attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getSize() {
	return getAttribute(HTMLConstants.ATTR_SIZE);
    }

    public void setSize(String size) {
	setAttribute(HTMLConstants.ATTR_SIZE, size);
    }

    /**
     *  The width of the rule. See the  width attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }

}

