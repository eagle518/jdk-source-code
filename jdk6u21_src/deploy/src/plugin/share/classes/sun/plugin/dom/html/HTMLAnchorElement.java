/*
 * @(#)HTMLAnchorElement.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;

/**
 *  A class that encapsulates an anchor element. 
 */
public final class HTMLAnchorElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLAnchorElement
{
    /**
     * Construct a HTMLAnchorElement object.
     */
    public HTMLAnchorElement(DOMObject obj, 
				org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
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
     *  Comma-separated list of lengths, defining an active region geometry. 
     * See also <code>shape</code> for the shape of the region. See the  
     * coords attribute definition in HTML 4.0.
     */
    public String getCoords() {
	return getAttribute(HTMLConstants.ATTR_COORDS);
    }

    public void setCoords(String coords) {
	setAttribute(HTMLConstants.ATTR_COORDS, coords);
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
     *  Anchor name. See the  name attribute definition in HTML 4.0.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  Forward link type. See the  rel attribute definition in HTML 4.0.
     */
    public String getRel() {
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
     *  The shape of the active area. The coordinates are given by 
     * <code>coords</code> . See the  shape attribute definition in HTML 4.0.
     */
    public String getShape() {
	return getAttribute(HTMLConstants.ATTR_SHAPE);
    }

    public void setShape(String shape) {
	setAttribute(HTMLConstants.ATTR_SHAPE, shape);
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

    /**
     *  Removes keyboard focus from this element.
     */
    public void blur() {
	obj.call(HTMLConstants.FUNC_BLUR, new Object[0]);
    }

    /**
     *  Gives keyboard focus to this element.
     */
    public void focus() {
	obj.call(HTMLConstants.FUNC_FOCUS, new Object[0]);
    }
}

