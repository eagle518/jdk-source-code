/*
 * @(#)HTMLInputElement.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;
/**
 *  A class that encapsulates an input element. 
 */
public class HTMLInputElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLInputElement
{
    /**
     * Construct a HTMLInputElement object.
     */
    public HTMLInputElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }
    
    /**
     *  When the <code>type</code> attribute of the element has the value 
     * "Text", "File" or "Password", this represents the HTML value attribute 
     * of the element. The value of this attribute does not change if the 
     * contents of the corresponding form control, in an interactive user 
     * agent, changes. Changing this attribute, however, resets the contents 
     * of the form control. See the  value attribute definition in HTML 4.0.
     */
    public String getDefaultValue() {
	return getAttribute(HTMLConstants.ATTR_DEFAULT_VALUE);
    }

    public void setDefaultValue(String defaultValue) {
	setAttribute(HTMLConstants.ATTR_DEFAULT_VALUE, defaultValue);
    }

    /**
     *  When <code>type</code> has the value "Radio" or "Checkbox", this 
     * represents the HTML checked attribute of the element. The value of 
     * this attribute does not change if the state of the corresponding form 
     * control, in an interactive user agent, changes. Changes to this 
     * attribute, however, resets the state of the form control. See the  
     * checked attribute definition in HTML 4.0.
     */
    public boolean getDefaultChecked() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DEFAULT_CHECKED);
    }

    public void setDefaultChecked(boolean defaultChecked) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DEFAULT_CHECKED, defaultChecked);
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
     *  A comma-separated list of content types that a server processing this 
     * form will handle correctly. See the  accept attribute definition in 
     * HTML 4.0.
     */
    public String getAccept() {
	return getAttribute(HTMLConstants.ATTR_ACCEPT);
    }

    public void setAccept(String accept) {
	setAttribute(HTMLConstants.ATTR_ACCEPT, accept);
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
     *  Aligns this object (vertically or horizontally)  with respect to its 
     * surrounding text. See the  align attribute definition in HTML 4.0. 
     * This attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /**
     *  Alternate text for user agents not rendering the normal content of 
     * this element. See the  alt attribute definition in HTML 4.0.
     */
    public String getAlt() {
	return getAttribute(HTMLConstants.ATTR_ALT);
    }

    public void setAlt(String alt) {
	setAttribute(HTMLConstants.ATTR_ALT, alt);
    }

    /**
     *  When the <code>type</code> attribute of the element has the value 
     * "Radio" or "Checkbox", this represents the current state of the form 
     * control, in an interactive user agent. Changes to this attribute 
     * change the state of the form control, but do not change the value of 
     * the HTML value attribute of the element.
     */
    public boolean getChecked() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_CHECKED);
    }

    public void setChecked(boolean checked) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_CHECKED, checked);
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
     *  Maximum number of characters for text fields, when <code>type</code> 
     * has the value "Text" or "Password". See the  maxlength attribute 
     * definition in HTML 4.0.
     */
    public int getMaxLength() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_MAX_LENGTH);
    }

    public void setMaxLength(int maxLength) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_MAX_LENGTH, maxLength);
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
     *  This control is read-only. Relevant only when <code>type</code> has 
     * the value "Text" or "Password". See the  readonly attribute definition 
     * in HTML 4.0.
     */
    public boolean getReadOnly() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_READONLY);
    }

    public void setReadOnly(boolean readOnly) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_READONLY, readOnly);
    }


    /**
     *  Size information. The precise meaning is specific to each type of 
     * field.  See the  size attribute definition in HTML 4.0.
     */
    public String getSize() {
	return getAttribute(HTMLConstants.ATTR_SIZE);
    }

    public void setSize(String size) {
	setAttribute(HTMLConstants.ATTR_SIZE, size);
    }


    /**
     *  When the <code>type</code> attribute has the value "Image", this 
     * attribute specifies the location of the image to be used to decorate 
     * the graphical submit button. See the  src attribute definition in HTML 
     * 4.0.
     */
    public String getSrc() {
	return getAttribute(HTMLConstants.ATTR_SRC);
    }

    public void setSrc(String src) {
	setAttribute(HTMLConstants.ATTR_SRC, src);
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
     *  The type of control created. See the  type attribute definition in 
     * HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    /**
     *  Use client-side image map. See the  usemap attribute definition in 
     * HTML 4.0.
     */
    public String getUseMap() {
	return getAttribute(HTMLConstants.ATTR_USE_MAP);
    }

    public void setUseMap(String useMap) {
	setAttribute(HTMLConstants.ATTR_USE_MAP, useMap);
    }

    /**
     *  When the <code>type</code> attribute of the element has the value 
     * "Text", "File" or "Password", this represents the current contents of 
     * the corresponding form control, in an interactive user agent. Changing 
     * this attribute changes the contents of the form control, but does not 
     * change the value of the HTML value attribute of the element. When the 
     * <code>type</code> attribute of the element has the value "Button", 
     * "Hidden", "Submit", "Reset", "Image", "Checkbox" or "Radio", this 
     * represents the HTML value attribute of the element. See the  value 
     * attribute definition in HTML 4.0.
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
     *  Select the contents of the text area. For <code>INPUT</code> elements 
     * whose <code>type</code> attribute has one of the following values: 
     * "Text", "File", or "Password".
     */
    public void select() {
	obj.call(HTMLConstants.FUNC_SELECT, null);
    }

    /**
     *  Simulate a mouse-click. For <code>INPUT</code> elements whose
     * <code>type</code> attribute has one of the following values: "Button", 
     * "Checkbox", "Radio", "Reset", or "Submit".
     */
    public void click() {
	obj.call(HTMLConstants.FUNC_CLICK, null);
    }

}

