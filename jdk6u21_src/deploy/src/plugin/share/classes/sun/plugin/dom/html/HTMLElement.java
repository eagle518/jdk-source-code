/*
 * @(#)HTMLElement.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.DOMException;
import sun.plugin.dom.*;
import sun.plugin.dom.core.Element;

/**
 *  A class that encapsulates any HTML element.
 */
public class HTMLElement extends sun.plugin.dom.core.Element 
			 implements org.w3c.dom.html.HTMLElement,
				    org.w3c.dom.css.ElementCSSInlineStyle {
    /**
     * Construct a HTMLElement object.
     */
    public HTMLElement(DOMObject obj, 
		       org.w3c.dom.html.HTMLDocument doc) { 
	super(obj, doc);
    }

    /**
     *  The element's identifier. See the  id attribute definition in HTML 4.0.
     */
    public String getId() {
	return getAttribute(HTMLConstants.ATTR_ID);	
    }

    public void setId(String id) {
	setAttribute(HTMLConstants.ATTR_ID, id);
    }

    /**
     *  The element's advisory title. See the  title attribute definition in 
     * HTML 4.0.
     */
    public String getTitle() {
	return getAttribute(HTMLConstants.ATTR_TITLE);
    }

    public void setTitle(String title) {
	setAttribute(HTMLConstants.ATTR_TITLE, title);
    }

    /**
     *  Language code defined in RFC 1766. See the  lang attribute definition 
     * in HTML 4.0.
     */
    public String getLang() {
	return getAttribute(HTMLConstants.ATTR_LANG);
    }

    public void setLang(String lang) {
	setAttribute(HTMLConstants.ATTR_LANG, lang);
    }

    /**
     *  Specifies the base direction of directionally neutral text and the 
     * directionality of tables. See the  dir attribute definition in HTML 
     * 4.0.
     */
    public String getDir() {
	return getAttribute(HTMLConstants.ATTR_DIR);
    }

    public void setDir(String dir) {
	setAttribute(HTMLConstants.ATTR_DIR, dir);
    }

    /**
     *  The class attribute of the element. This attribute has been renamed 
     * due to conflicts with the "class" keyword exposed by many languages. 
     * See the  class attribute definition in HTML 4.0.
     */
    public String getClassName() {
	return getAttribute(HTMLConstants.ATTR_CLASSNAME);
    }

    public void setClassName(String className) {
	setAttribute(HTMLConstants.ATTR_CLASSNAME, className);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSSStyleDeclaration
    //------------------------------------------------------------

    /**
     *  The style attribute. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getStyle() {
        return DOMObjectFactory.createCSSStyleDeclaration(obj.getMember(HTMLConstants.MEMBER_STYLE),
                                                          getOwnerDocument());
    }
}
