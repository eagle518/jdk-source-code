/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

public final class HTMLAreaElement extends HTMLElement
			     implements org.w3c.dom.html.HTMLAreaElement {

    public HTMLAreaElement(DOMObject obj, 
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
     *  Specifies that this area is inactive, i.e., has no associated action. 
     * See the  nohref attribute definition in HTML 4.0.
     */
    public boolean getNoHref() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_NO_HREF);
    }

    public void setNoHref(boolean noHref) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_NO_HREF, noHref);
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

    public void setTabIndex(int tabIndex){
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
}
