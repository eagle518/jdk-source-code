/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.DOMObject;

public class HTMLBodyElement extends sun.plugin.dom.html.HTMLElement 
			         implements org.w3c.dom.html.HTMLBodyElement {

    public HTMLBodyElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /* Color of active links (after mouse-button down, but before mouse-button up). 
	See the alink attribute definition in HTML 4.01. 
	This attribute is deprecated in HTML 4.01.
    */
    public String getALink() {
	return getAttribute(HTMLConstants.ATTR_ALINK);
    }

    public void setALink(String aLink) {
	setAttribute(HTMLConstants.ATTR_ALINK, aLink);
    }

    /* URI [IETF RFC 2396] of the background texture tile image. 
	See the background attribute definition in HTML 4.01. 
	This attribute is deprecated in HTML 4.01.
    */
    public String getBackground() {
	return getAttribute(HTMLConstants.ATTR_BACKGROUND);
    }

    public void setBackground(String background) {
	setAttribute(HTMLConstants.ATTR_BACKGROUND, background);
    }

    /* Document background color. See the bgcolor attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
    */
    public String getBgColor() {
	return getAttribute(HTMLConstants.ATTR_BGCOLOR);
    }

    public void setBgColor(String bgColor) {
	setAttribute(HTMLConstants.ATTR_BGCOLOR, bgColor);
    }
    
    /* Color of links that are not active and unvisited. 
     * See the link attribute definition in HTML 4.01. This attribute is deprecated in HTML 4.01.
     */
    public String getLink() {
	return getAttribute(HTMLConstants.ATTR_LINK);
    }

    public void setLink(String link) {
	setAttribute(HTMLConstants.ATTR_LINK, link);
    }

    /* Document text color. See the text attribute definition in HTML 4.01. 
     * This attribute is deprecated in HTML 4.01.
     */
    public String getText() {
	return getAttribute(HTMLConstants.ATTR_TEXT);
    }

    public void setText(String text) {
	setAttribute(HTMLConstants.ATTR_TEXT, text);
    }
    
    /* Color of links that have been visited by the user. 
     * See the vlink attribute definition in HTML 4.01. This attribute is deprecated in HTML 4.01
     */
    public String getVLink() {
	return getAttribute(HTMLConstants.ATTR_VLINK);
    }

    public void setVLink(String vLink) {
	setAttribute(HTMLConstants.ATTR_VLINK, vLink);
    }
}
