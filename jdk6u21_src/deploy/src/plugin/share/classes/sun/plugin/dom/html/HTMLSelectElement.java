/*
 * @(#)HTMLSelectElement.java	1.8 10/03/24
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
 *  A class that encapsulates an select element. 
 */
public class HTMLSelectElement extends HTMLElement
			       implements org.w3c.dom.html.HTMLSelectElement
{
    /**
     * Construct a HTMLSelectElement object.
     */
    public HTMLSelectElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }
    
    /**
     *  The type of this form control. This is the string "select-multiple" 
     * when the multiple attribute is <code>true</code> and the string 
     * "select-one" when <code>false</code> .
     */
    public String getType()
    {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    /**
     *  The ordinal index of the selected option, starting from 0. The value 
     * -1 is returned if no element is selected. If multiple options are 
     * selected, the index of the first selected option is returned. 
     */
    public int getSelectedIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_SELECTED_INDEX);
    }

    public void setSelectedIndex(int selectedIndex) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_SELECTED_INDEX, selectedIndex);
    }

    /**
     *  The current form control value. 
     */
    public String getValue() {
	return getAttribute(HTMLConstants.ATTR_VALUE);
    }

    public void setValue(String value) {
	setAttribute(HTMLConstants.ATTR_VALUE, value);
    }

    /**
     *  The number of options in this <code>SELECT</code> . 
     */
    public int getLength() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_LENGTH);
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
     *  The collection of <code>OPTION</code> elements contained by this 
     * element. 
     */
    public org.w3c.dom.html.HTMLCollection getOptions() {
        // Note: IE workarounds removed here; need to re-test
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.ATTR_OPTIONS),
                                                     (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    private org.w3c.dom.html.HTMLOptionElement getOptionItem(int index) {
	return DOMObjectFactory.createHTMLOptionElement(obj.call(HTMLConstants.FUNC_ITEM, new Object[]{new Integer(index)}),
                                                        (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
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
     *  If true, multiple <code>OPTION</code> elements may  be selected in 
     * this <code>SELECT</code> . See the  multiple attribute definition in 
     * HTML 4.0.
     */
    public boolean getMultiple() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_MULTIPLE);
    }

    public void setMultiple(boolean multiple) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_MULTIPLE, multiple);
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
     *  Number of visible rows. See the  size attribute definition in HTML 4.0.
     */
    public int getSize() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_SIZE);
    }

    public void setSize(int size) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_SIZE, size);
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
     *  Add a new element to the collection of <code>OPTION</code> elements 
     * for this <code>SELECT</code> . This method is the equivalent of the 
     * <code>appendChild</code> method of the <code>Node</code> interface if 
     * the <code>before</code> parameter is <code>null</code> . It is 
     * equivalent to the <code>insertBefore</code> method on the parent of 
     * <code>before</code> in all other cases.
     * @param element  The element to add.
     * @param before  The element to insert before, or <code>null</code> for 
     *   the tail of the list.
     * @exception DOMException
     *    NOT_FOUND_ERR: Raised if <code>before</code> is not a descendant of 
     *   the <code>SELECT</code> element. 
     */
    public void add(org.w3c.dom.html.HTMLElement element, 
                    org.w3c.dom.html.HTMLElement before)
                    throws DOMException {
        obj.call(HTMLConstants.FUNC_ADD,
                 new Object[] { element, before });
    }


    /**
     *  Remove an element from the collection of <code>OPTION</code> elements 
     * for this <code>SELECT</code> . Does nothing if no element has the given
     *  index.
     * @param index  The index of the item to remove, starting from 0.
     */
    public void remove(int index) {
	Object args[] = new Object[1];
	args[0] = new Integer(index);

	obj.call(HTMLConstants.FUNC_REMOVE, args);
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
}

