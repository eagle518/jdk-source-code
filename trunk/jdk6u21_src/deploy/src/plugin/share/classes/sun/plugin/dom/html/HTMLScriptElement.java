/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Script statements. See the  SCRIPT element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLScriptElement extends HTMLElement 
				     implements org.w3c.dom.html.HTMLScriptElement {

    public HTMLScriptElement(DOMObject obj, 
			     org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The script content of the element. 
     */
    public String getText() {
	return getAttribute(HTMLConstants.ATTR_TEXT);
    }

    public void setText(String text) {
	setAttribute(HTMLConstants.ATTR_TEXT, text);
    }

    /**
     *  Reserved for future use. 
     */
    public String getHtmlFor() {
	return getAttribute(HTMLConstants.ATTR_HTML_FOR);
    }

    public void setHtmlFor(String htmlFor) {
	setAttribute(HTMLConstants.ATTR_HTML_FOR, htmlFor);
    }

    /**
     *  Reserved for future use. 
     */
    public String getEvent() {
	return getAttribute(HTMLConstants.ATTR_EVENT);
    }

    public void setEvent(String event) {
	setAttribute(HTMLConstants.ATTR_EVENT, event);
    }

    /**
     *  The character encoding of the linked resource. See the  charset 
     * attribute definition in HTML 4.0.
     */
    public String getCharset() {
	return getAttribute(HTMLConstants.ATTR_CHARSET);
    }

    public void setCharset(String charset) {
	setAttribute(HTMLConstants.ATTR_CHARSET, charset);
    }

    /**
     *  Indicates that the user agent can defer processing of the script.  See 
     * the  defer attribute definition in HTML 4.0.
     */
    public boolean getDefer() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DEFER);
    }

    public void setDefer(boolean defer) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DEFER, defer);
    }

    /**
     *  URI designating an external script. See the  src attribute definition 
     * in HTML 4.0.
     */
    public String getSrc() {
	return getAttribute(HTMLConstants.ATTR_SRC);
    }

    public void setSrc(String src) {
	setAttribute(HTMLConstants.ATTR_SRC, src);
    }

    /**
     *  The content type of the script language. See the  type attribute 
     * definition in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

}

