/*
 * @(#)HTMLFormElement.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;

/**
 *  The <code>FORM</code> element encompasses behavior similar to a collection 
 * and an element. It provides direct access to the contained input elements 
 * as well as the attributes of the form element. See the  FORM element 
 * definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLFormElement extends sun.plugin.dom.html.HTMLElement 
			     implements org.w3c.dom.html.HTMLFormElement
{
    /**
     * Construct a HTMLFormElement object.
     */
    public HTMLFormElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Returns a collection of all control elements in the form. 
     */
    public org.w3c.dom.html.HTMLCollection getElements() {
	return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_ELEMENTS),
                                                     (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    /**
     *  The number of form controls in the form.
     */
    public int getLength() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_LENGTH);
    }

    /**
     *  Names the form. 
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  List of character sets supported by the server. See the  
     * accept-charset attribute definition in HTML 4.0.
     */
    public String getAcceptCharset() {
	return getAttribute(HTMLConstants.ATTR_ACCEPT_CHARSET);
    }

    public void setAcceptCharset(String acceptCharset) {
	setAttribute(HTMLConstants.ATTR_ACCEPT_CHARSET, acceptCharset);
    }

    /**
     *  Server-side form handler. See the  action attribute definition in HTML 
     * 4.0.
     */
    public String getAction() {
	return getAttribute(HTMLConstants.ATTR_ACTION);
    }

    public void setAction(String action) {
	setAttribute(HTMLConstants.ATTR_ACTION, action);
    }

    /**
     *  The content type of the submitted form,  generally 
     * "application/x-www-form-urlencoded".  See the  enctype attribute 
     * definition in HTML 4.0.
     */
    public String getEnctype() {
	return getAttribute(HTMLConstants.ATTR_ENC_TYPE);
    }

    public void setEnctype(String enctype) {
	setAttribute(HTMLConstants.ATTR_ENC_TYPE, enctype);
    }

    /**
     *  HTTP method used to submit form. See the  method attribute definition 
     * in HTML 4.0.
     */
    public String getMethod() {
	return getAttribute(HTMLConstants.ATTR_METHOD);
    }

    public void setMethod(String method) {
	setAttribute(HTMLConstants.ATTR_METHOD, method);
    }

    /**
     *  Frame to render the resource in. See the  target attribute definition 
     * in HTML 4.0.
     */
    public String getTarget() {
	return getAttribute(HTMLConstants.ATTR_TARGET);
    }

    public void setTarget(String target) {
	setAttribute(HTMLConstants.ATTR_TARGET, target);
    }

    /**
     *  Submits the form. It performs the same action as a  submit button.
     */
    public void submit() {
	obj.call(HTMLConstants.FUNC_SUBMIT, new Object[0]);
    }

    /**
     *  Restores a form element's default values. It performs  the same action 
     * as a reset button.
     */
    public void reset() {
	obj.call(HTMLConstants.FUNC_RESET, new Object[0]);
    }
}

