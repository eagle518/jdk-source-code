/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;


/**
 *  Group options together in logical subdivisions. See the  OPTGROUP element 
 * definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLOptGroupElement extends HTMLElement 
				       implements org.w3c.dom.html.HTMLOptGroupElement {
    
    public HTMLOptGroupElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The control is unavailable in this context. See the  disabled 
     * attribute definition in HTML 4.0.
     */
    public boolean getDisabled() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DISABLED);
    }

    public void setDisabled(boolean disabled) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DISABLED, disabled);
    }

    /**
     *  Assigns a label to this option group. See the  label attribute 
     * definition in HTML 4.0.
     */
    public String getLabel() {
	return getAttribute(HTMLConstants.ATTR_LABEL);
    }

    public void setLabel(String label) {
	setAttribute(HTMLConstants.ATTR_LABEL, label);
    }

}

