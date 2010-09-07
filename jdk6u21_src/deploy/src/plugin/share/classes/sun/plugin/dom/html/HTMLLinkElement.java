/*
 * @(#)HTMLLinkElement.java	1.5 10/03/24
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
 *  The <code>LINK</code> element specifies a link to an external resource, 
 * and defines this document's relationship to that resource (or vice versa). 
 *  See the  LINK element definition in HTML 4.0  (see also the 
 * <code>LinkStyle</code> interface in the  module).
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
/**
 *  Style information. See the  STYLE element definition in HTML 4.0, the  
 * module and the <code>LinkStyle</code> interface in the  module. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLLinkElement extends sun.plugin.dom.html.HTMLElement 
			          implements org.w3c.dom.html.HTMLLinkElement, 
					     org.w3c.dom.html.HTMLStyleElement,
					     org.w3c.dom.stylesheets.LinkStyle
{
    /**
     * Construct a HTMLObjectElement object.
     */
    public HTMLLinkElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Enables/disables the link. This is currently only used for style sheet 
     * links, and may be used to activate or deactivate style sheets. 
     */
    public boolean getDisabled() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DISABLED);
    }

    public void setDisabled(boolean disabled) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DISABLED, disabled);
    }

    /**
     *  The character encoding of the resource being linked to. See the  
     * charset attribute definition in HTML 4.0.
     */
    public String getCharset() {
	return getAttribute(HTMLConstants.ATTR_CHARSET);
    }

    public void setCharset(String charset) {
	setAttribute(HTMLConstants.ATTR_CHARSET, charset);
    }

    /**
     *  The URI of the linked resource. See the  href attribute definition in 
     * HTML 4.0.
     */
    public String getHref() {
	return getAttribute(HTMLConstants.ATTR_HREF);
    }

    public void setHref(String href) {
	setAttribute(HTMLConstants.ATTR_HREF, href);
    }

    /**
     *  Language code of the linked resource. See the  hreflang attribute 
     * definition in HTML 4.0.
     */
    public String getHreflang() {
	return getAttribute(HTMLConstants.ATTR_HREF_LANG);
    }

    public void setHreflang(String hreflang) {
	setAttribute(HTMLConstants.ATTR_HREF_LANG, hreflang);
    }


    /**
     *  Designed for use with one or more target media. See the  media 
     * attribute definition in HTML 4.0.
     */
    public String getMedia() {
	return getAttribute(HTMLConstants.ATTR_MEDIA);
    }

    public void setMedia(String media) {
	setAttribute(HTMLConstants.ATTR_MEDIA, media);
    }

    /**
     *  Forward link type. See the  rel attribute definition in HTML 4.0.
     */
    public String getRel(){
	return getAttribute(HTMLConstants.ATTR_REL);
    }

    public void setRel(String rel) {
	setAttribute(HTMLConstants.ATTR_REL, rel);
    }

    /**
     *  Reverse link type. See the  rev attribute definition in HTML 4.0.
     */
    public String getRev() {
	return getAttribute(HTMLConstants.ATTR_REV);
    }

    public void setRev(String rev) {
	setAttribute(HTMLConstants.ATTR_REV, rev);
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
     *  Advisory content type. See the  type attribute definition in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

    //------------------------------------------------------------
    // Method from org.w3c.dom.stylesheets.LinkStyle
    //------------------------------------------------------------

    /**
     *  The style sheet. 
     */
    public org.w3c.dom.stylesheets.StyleSheet getSheet() {
	Object result = null;

	try {
	    result = obj.getMember(HTMLConstants.MEMBER_STYLESHEET);
	} catch (DOMException e) {
	}

	if (result == null) {
	    try {
		result = obj.getMember(HTMLConstants.MEMBER_SHEET);
	    }catch (DOMException e) {
	    }
	}

	if (result != null && result instanceof DOMObject) {
	    return (org.w3c.dom.stylesheets.StyleSheet)DOMObjectFactory.createStyleSheet((DOMObject)result, getOwnerDocument());
	}
	return null;
    }
}

