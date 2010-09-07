/*
 * @(#)HTMLFrameElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Document;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Create a frame. See the  FRAME element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLFrameElement extends HTMLElement 
					    implements org.w3c.dom.html.HTMLFrameElement {
    public HTMLFrameElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Request frame borders. See the  frameborder attribute definition in 
     * HTML 4.0.
     */
    public String getFrameBorder() {
	return getAttribute(HTMLConstants.ATTR_FRAME_BORDER);
    }

    public void setFrameBorder(String frameBorder) {
	setAttribute(HTMLConstants.ATTR_FRAME_BORDER, frameBorder);
    }

    /**
     *  URI designating a long description of this image or frame. See the  
     * longdesc attribute definition in HTML 4.0.
     */
    public String getLongDesc() {
	return getAttribute(HTMLConstants.ATTR_LONGDESC);
    }

    public void setLongDesc(String longDesc) {
	setAttribute(HTMLConstants.ATTR_LONGDESC, longDesc);
    }

    /**
     *  Frame margin height, in pixels. See the  marginheight attribute 
     * definition in HTML 4.0.
     */
    public String getMarginHeight() {
	return getAttribute(HTMLConstants.ATTR_MARGIN_HEIGHT);
    }

    public void setMarginHeight(String marginHeight) {
	setAttribute(HTMLConstants.ATTR_MARGIN_HEIGHT, marginHeight);
    }

    /**
     *  Frame margin width, in pixels. See the  marginwidth attribute 
     * definition in HTML 4.0.
     */
    public String getMarginWidth() {
	return getAttribute(HTMLConstants.ATTR_MARGIN_WIDTH);
    }

    public void setMarginWidth(String marginWidth) {
	setAttribute(HTMLConstants.ATTR_MARGIN_WIDTH, marginWidth);
    }

    /**
     *  The frame name (object of the <code>target</code> attribute). See the  
     * name attribute definition in HTML 4.0.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  When true, forbid user from resizing frame. See the  noresize 
     * attribute definition in HTML 4.0.
     */
    public boolean getNoResize() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_NO_RESIZE);
    }

    public void setNoResize(boolean noResize) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_NO_RESIZE, noResize);
    }

    /**
     *  Specify whether or not the frame should have scrollbars. See the  
     * scrolling attribute definition in HTML 4.0.
     */
    public String getScrolling() {
	return getAttribute(HTMLConstants.ATTR_SCROLLING);
    }

    public void setScrolling(String scrolling) {
	setAttribute(HTMLConstants.ATTR_SCROLLING, scrolling);
    }

    /**
     *  A URI designating the initial frame contents. See the  src attribute 
     * definition in HTML 4.0.
     */
    public String getSrc() {
	return getAttribute(HTMLConstants.ATTR_SRC);
    }

    public void setSrc(String src) {
	setAttribute(HTMLConstants.ATTR_SRC, src);
    }

    /**
     *  The document this frame contains, if there is any and it is available, 
     * or <code>null</code> otherwise.
     * @since DOM Level 2
     */
    public Document getContentDocument() {
        Object res = obj.getMember(HTMLConstants.ATTR_CONTENT_DOCUMENT);
        if (res != null && res instanceof DOMObject) {
            return new HTMLDocument((DOMObject) res, null);
        }
        return null;
    }
}
