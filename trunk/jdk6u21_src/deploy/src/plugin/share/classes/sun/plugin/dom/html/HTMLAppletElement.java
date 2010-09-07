/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

public final class HTMLAppletElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLAppletElement {

    public HTMLAppletElement(DOMObject obj, 
			     org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /* Aligns this object (vertically or horizontally) with respect to its surrounding text. 
     * See the align attribute definition in HTML 4.01. This attribute is deprecated in HTML 4.01.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /* Alternate text for user agents not rendering the normal content of this element. 
     * See the alt attribute definition in HTML 4.01. This attribute is deprecated in HTML 4.01.
     */
    public String getAlt() {
	return getAttribute(HTMLConstants.ATTR_ALT);
    }

    public void setAlt(String alt) {
	setAttribute(HTMLConstants.ATTR_ALT, alt);
    }

    /* Comma-separated archive list. 
     * See the archive attribute definition in HTML 4.01. This attribute is deprecated in HTML 4.01.
     */
    public String getArchive() {
	return getAttribute(HTMLConstants.ATTR_ARCHIVE);
    }

    public void setArchive(String archive) {
	setAttribute(HTMLConstants.ATTR_ARCHIVE, archive);
    }

    /* Applet class file. See the code attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getCode() {
	return getAttribute(HTMLConstants.ATTR_CODE);
    }

    public void setCode(String code) {
	setAttribute(HTMLConstants.ATTR_CODE, code);
    }

    /* Optional base URI [IETF RFC 2396] for applet. 
     * See the codebase attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getCodeBase() {
	return getAttribute(HTMLConstants.ATTR_CODEBASE);
    }

    public void setCodeBase(String codeBase) {
	setAttribute(HTMLConstants.ATTR_CODEBASE, codeBase);
    }
    
    /* Override height. See the height attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getHeight() {
	return getAttribute(HTMLConstants.ATTR_HEIGHT);
    }

    public void setHeight(String height) {
	setAttribute(HTMLConstants.ATTR_HEIGHT, height);
    }

    /* Horizontal space, in pixels, to the left and right of this image, applet, or object. 
     * See the hspace attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getHspace() {
	return getAttribute(HTMLConstants.ATTR_HSPACE);
    }

    public void setHspace(String hspace) {
	setAttribute(HTMLConstants.ATTR_HSPACE, hspace);
    }

    /* The name of the applet. See the name attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }
    
    /**
     *  Serialized applet file. See the  object attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getObject() {
	return getAttribute(HTMLConstants.ATTR_OBJECT);
    }

    public void setObject(String object) {
	setAttribute(HTMLConstants.ATTR_OBJECT, object);
    }

    /**
     *  Vertical space above and below this image, applet, or object. See the  
     * vspace attribute definition in HTML 4.0. This attribute is deprecated 
     * in HTML 4.0.
     */
    public String getVspace() {
	return getAttribute(HTMLConstants.ATTR_VSPACE);
    }

    public void setVspace(String vspace) {
	setAttribute(HTMLConstants.ATTR_VSPACE, vspace);
    }

    /**
     *  Override width. See the  width attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }
}
