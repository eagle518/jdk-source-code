/*
 * @(#)HTMLOptionElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  A selectable choice. See the  OPTION element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLOptionElement extends HTMLElement 
				     implements org.w3c.dom.html.HTMLOptionElement{

    public HTMLOptionElement(DOMObject obj, 
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
     *  Represents the value of the HTML selected attribute. The value of this 
     * attribute does not change if the state of the corresponding form 
     * control, in an interactive user agent, changes. Changing 
     * <code>defaultSelected</code> , however, resets the state of the form 
     * control. See the  selected attribute definition in HTML 4.0.
     */
    public boolean getDefaultSelected() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DEFAULT_SELECTED);
    }

    public void setDefaultSelected(boolean defaultSelected) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DEFAULT_SELECTED, defaultSelected);
    }

    /**
     *  The text contained within the option element. 
     */
    public String getText() {
	return getAttribute(HTMLConstants.ATTR_TEXT);
    }

    /**
     *  The index of this <code>OPTION</code> in its parent <code>SELECT</code>
     *  , starting from 0.
     */
    public int getIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_INDEX);
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
     *  Option label for use in hierarchical menus. See the  label attribute 
     * definition in HTML 4.0.
     */
    public String getLabel() {
	return getAttribute(HTMLConstants.ATTR_LABEL);
    }

    public void setLabel(String label) {
	setAttribute(HTMLConstants.ATTR_LABEL, label);
    }

    /**
     *  Represents the current state of the corresponding form control, in an 
     * interactive user agent. Changing this attribute changes the state of 
     * the form control, but does not change the value of the HTML selected 
     * attribute of the element.
     */
    public boolean getSelected() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_SELECTED);
    }

    public void setSelected(boolean selected) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_SELECTED, selected);
    }

    /**
     *  The current form control value. See the  value attribute definition in 
     * HTML 4.0.
     */
    public String getValue() {
	return getAttribute(HTMLConstants.ATTR_VALUE);
    }

    public void setValue(String value) {
	setAttribute(HTMLConstants.ATTR_VALUE, value);
    }
}

