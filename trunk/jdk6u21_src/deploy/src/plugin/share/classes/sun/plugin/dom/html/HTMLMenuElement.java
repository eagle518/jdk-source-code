/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Element;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Menu list. See the  MENU element definition in HTML 4.0. This element is 
 * deprecated in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLMenuElement extends HTMLElement
				   implements org.w3c.dom.html.HTMLMenuElement {

    public HTMLMenuElement(DOMObject obj, 
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

}

