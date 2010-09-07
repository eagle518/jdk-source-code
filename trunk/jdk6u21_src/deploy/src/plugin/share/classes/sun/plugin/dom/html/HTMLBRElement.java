/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html;

import org.w3c.dom.Element;
import sun.plugin.dom.DOMObject;

public class HTMLBRElement extends HTMLElement
			   implements org.w3c.dom.html.HTMLBRElement {

    public HTMLBRElement(DOMObject obj, 
			 org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Control flow of text around floats. See the  clear attribute definition
     *  in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getClear() {
	return getAttribute(HTMLConstants.ATTR_CLEAR);
    }

    public void setClear(String clear) {
	setAttribute(HTMLConstants.ATTR_CLEAR, clear);
    }
}
