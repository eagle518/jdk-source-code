/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

public class HTMLBaseElement extends HTMLElement
			     implements org.w3c.dom.html.HTMLBaseElement {

    public HTMLBaseElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The base URI. See the  href attribute definition in HTML 4.0.
     */
    public String getHref() {
	return getAttribute(HTMLConstants.ATTR_HREF);
    }

    public void setHref(String href) {
	setAttribute(HTMLConstants.ATTR_HREF, href);
    }

    /**
     *  The default target frame. See the  target attribute definition in HTML 
     * 4.0.
     */
    public String getTarget() {
	return getAttribute(HTMLConstants.ATTR_TARGET);
    }

    public void setTarget(String target) {
	setAttribute(HTMLConstants.ATTR_TARGET, target);
    }
}
