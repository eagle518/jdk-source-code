/*
 * @(#)HTMLTextAreaElement.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  A class that encapsulates an input element. 
 */
public class HTMLTextAreaElement extends sun.plugin.dom.html.HTMLElement 
			         implements org.w3c.dom.html.HTMLTextAreaElement
{
    /**
     * Construct a HTMLTextAreaElement object.
     */
    public HTMLTextAreaElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Represents the contents of the element. The value of this attribute 
     * does not change if the contents of the corresponding form control, in 
     * an interactive user agent, changes. Changing this attribute, however, 
     * resets the contents of the form control.
     */
    public String getDefaultValue() {	
	return getAttribute(HTMLConstants.ATTR_DEFAULT_VALUE);
    }

    public void setDefaultValue(String defaultValue) {
	setAttribute(HTMLConstants.ATTR_DEFAULT_VALUE, defaultValue);
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
     *  Width of control (in characters). See the  cols attribute definition 
     * in HTML 4.0.
     */
    public int getCols() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_COLS);
    }

    public void setCols(int cols) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_COLS, cols);
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
     *  Form control or object name when submitted with a form. See the  name 
     * attribute definition in HTML 4.0.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  This control is read-only. See the  readonly attribute definition in 
     * HTML 4.0.
     */
    public boolean getReadOnly() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_READONLY);
    }

    public void setReadOnly(boolean readOnly) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_READONLY, readOnly);
    }

    /**
     *  Number of text rows. See the  rows attribute definition in HTML 4.0.
     */
    public int getRows() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_ROWS);
    }

    public void setRows(int rows) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_ROWS, rows);
    }
    
    /**
     *  Index that represents the element's position in the tabbing order. See 
     * the  tabindex attribute definition in HTML 4.0.
     */
    public int getTabIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_TAB_INDEX);
    }

    public void setTabIndex(int tabIndex) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_TAB_INDEX, tabIndex);
    }
    
    /**
     *  The type of this form control. This the string "textarea".
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    /**
     *  Represents the current contents of the corresponding form control, in 
     * an interactive user agent. Changing this attribute changes the 
     * contents of the form control, but does not change the contents of the 
     * element. If the entirety of the data can not fit into a single 
     * <code>DOMString</code> , the implementation may truncate the data.
     */
    public String getValue() {
	return getAttribute(HTMLConstants.ATTR_VALUE);
    }

    public void setValue(String value) {
	setAttribute(HTMLConstants.ATTR_VALUE, value);
    }

    /**
     *  Removes keyboard focus from this element.
     */
    public void blur() {
	obj.call(HTMLConstants.FUNC_BLUR, null);
    }

    /**
     *  Gives keyboard focus to this element.
     */
    public void focus() {
	obj.call(HTMLConstants.FUNC_FOCUS, null);
    }

    /**
     *  Select the contents of the <code>TEXTAREA</code> .
     */
    public void select() {
	obj.call(HTMLConstants.FUNC_SELECT, null);
    }
}

