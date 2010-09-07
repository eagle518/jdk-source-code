/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  List item. See the  LI element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLLIElement extends HTMLElement
				 implements org.w3c.dom.html.HTMLLIElement {

    public HTMLLIElement(DOMObject obj, 
			 org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  List item bullet style. See the  type attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

    /**
     *  Reset sequence number when used in <code>OL</code> . See the  value 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public int getValue() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_VALUE);
    }

    public void setValue(int value) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_VALUE, value);
    }

}

