/*
 * @(#)HTMLLegendElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  Provides a caption for a <code>FIELDSET</code> grouping.  See the  LEGEND 
 * element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLLegendElement extends HTMLElement
				     implements org.w3c.dom.html.HTMLLegendElement {

    public HTMLLegendElement(DOMObject obj, 
			     org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Returns the <code>FORM</code> element containing this control. Returns 
     * <code>null</code> if this control is not within the context of a form. 
     */
    public org.w3c.dom.html.HTMLFormElement getForm() {
    	return DOMObjectFactory.createHTMLFormElement(obj.getMember(HTMLConstants.ATTR_FORM),
                                                      (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    /**
     *  A single character access key to give access to the form control. See 
     * the  accesskey attribute definition in HTML 4.0.
     */
    public String getAccessKey() {
	return getAttribute(HTMLConstants.ATTR_ACCESS_KEY);
    }

    public void setAccessKey(String accessKey) {
	setAttribute(HTMLConstants.ATTR_ACCESS_KEY, accessKey);
    }

    /**
     *  Text alignment relative to <code>FIELDSET</code> . See the  align 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

}

