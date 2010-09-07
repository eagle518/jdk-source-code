/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Ordered list. See the  OL element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLOListElement extends HTMLElement 
				    implements org.w3c.dom.html.HTMLOListElement {

    public HTMLOListElement(DOMObject obj, 
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
     *  Starting sequence number. See the  start attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public int getStart() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_START);
    }

    public void setStart(int start) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_START, start);
    }

    /**
     *  Numbering style. See the  type attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }
}
