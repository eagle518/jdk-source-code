/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Unordered list. See the  UL element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLUListElement extends HTMLElement 
	implements org.w3c.dom.html.HTMLUListElement {

    public HTMLUListElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Reduce spacing between list items. See the  compact attribute 
     * definition in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public boolean getCompact() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_COMPACT);
    }

    public void setCompact(boolean compact) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_COMPACT, compact);
    }

    /**
     *  Bullet style. See the  type attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }
}

