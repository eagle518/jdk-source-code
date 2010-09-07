/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

/**
 *  Parameters fed to the <code>OBJECT</code> element. See the  PARAM element 
 * definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLParamElement extends HTMLElement 
				    implements org.w3c.dom.html.HTMLParamElement {
    
    public HTMLParamElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The name of a run-time parameter. See the  name attribute definition 
     * in HTML 4.0.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  Content type for the <code>value</code> attribute when
     * <code>valuetype</code> has the value "ref". See the  type attribute 
     * definition in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

    /**
     *  The value of a run-time parameter. See the  value attribute definition 
     * in HTML 4.0.
     */
    public String getValue() {
	return getAttribute(HTMLConstants.ATTR_VALUE);
    }

    public void setValue(String value) {
	setAttribute(HTMLConstants.ATTR_VALUE, value);
    }

    /**
     *  Information about the meaning of the <code>value</code> attribute 
     * value. See the  valuetype attribute definition in HTML 4.0.
     */
    public String getValueType() {
	return getAttribute(HTMLConstants.ATTR_VALUE_TYPE);
    }

    public void setValueType(String valueType) {
	setAttribute(HTMLConstants.ATTR_VALUE_TYPE, valueType);
    }

}

