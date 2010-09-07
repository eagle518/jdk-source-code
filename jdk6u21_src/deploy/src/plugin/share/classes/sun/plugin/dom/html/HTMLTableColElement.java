/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Regroups the <code>COL</code> and <code>COLGROUP</code> elements. See the  
 * COL element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTableColElement extends HTMLElement
				       implements org.w3c.dom.html.HTMLTableColElement {
    public HTMLTableColElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Horizontal alignment of cell data in column. See the  align attribute 
     * definition in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /**
     *  Alignment character for cells in a column. See the  char attribute 
     * definition in HTML 4.0.
     */
    public String getCh() {
	return getAttribute(HTMLConstants.ATTR_CH);
    }

    public void setCh(String ch) {
	setAttribute(HTMLConstants.ATTR_CH, ch);
    }

    /**
     *  Offset of alignment character. See the  charoff attribute definition 
     * in HTML 4.0.
     */
    public String getChOff() {
	return getAttribute(HTMLConstants.ATTR_CHOFF);
    }

    public void setChOff(String chOff) {
	setAttribute(HTMLConstants.ATTR_CHOFF, chOff);
    }

    /**
     *  Indicates the number of columns in a group or affected by a grouping. 
     * See the  span attribute definition in HTML 4.0.
     */
    public int getSpan() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_SPAN);
    }

    public void setSpan(int span) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_SPAN, span);
    }

    /**
     *  Vertical alignment of cell data in column. See the  valign attribute 
     * definition in HTML 4.0.
     */
    public String getVAlign() {
	return getAttribute(HTMLConstants.ATTR_VALIGN);
    }

    public void setVAlign(String vAlign) {
	setAttribute(HTMLConstants.ATTR_VALIGN, vAlign);
    }

    /**
     *  Default column width. See the  width attribute definition in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }

}

