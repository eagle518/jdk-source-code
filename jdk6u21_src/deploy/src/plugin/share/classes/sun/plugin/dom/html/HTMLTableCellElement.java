/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  The object used to represent the <code>TH</code> and <code>TD</code> 
 * elements. See the  TD element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTableCellElement extends HTMLElement 
					implements org.w3c.dom.html.HTMLTableCellElement {

    public HTMLTableCellElement(DOMObject obj, 
				org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The index of this cell in the row, starting from 0. This index is in 
     * document tree order and not display order.
     */
    public int getCellIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_CELL_INDEX);
    }

    /**
     *  Abbreviation for header cells. See the  abbr attribute definition in 
     * HTML 4.0.
     */
    public String getAbbr() {
	return getAttribute(HTMLConstants.ATTR_ABBR);
    }

    public void setAbbr(String abbr) {
	setAttribute(HTMLConstants.ATTR_ABBR, abbr);
    }

    /**
     *  Horizontal alignment of data in cell. See the  align attribute 
     * definition in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /**
     *  Names group of related headers. See the  axis attribute definition in 
     * HTML 4.0.
     */
    public String getAxis() {
	return getAttribute(HTMLConstants.ATTR_AXIS);
    }

    public void setAxis(String axis) {
	setAttribute(HTMLConstants.ATTR_AXIS, axis);
    }

    /**
     *  Cell background color. See the  bgcolor attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getBgColor() {
	return getAttribute(HTMLConstants.ATTR_BGCOLOR);
    }

    public void setBgColor(String bgColor) {
	setAttribute(HTMLConstants.ATTR_BGCOLOR, bgColor);
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
     *  Number of columns spanned by cell. See the  colspan attribute 
     * definition in HTML 4.0.
     */
    public int getColSpan() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_COL_SPAN);
    }

    public void setColSpan(int colSpan) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_COL_SPAN, colSpan);
    }

    /**
     *  List of <code>id</code> attribute values for header cells. See the  
     * headers attribute definition in HTML 4.0.
     */
    public String getHeaders() {
	return getAttribute(HTMLConstants.ATTR_HEADERS);
    }

    public void setHeaders(String headers) {
	setAttribute(HTMLConstants.ATTR_HEADERS, headers);
    }

    /**
     *  Cell height. See the  height attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getHeight() {
	return getAttribute(HTMLConstants.ATTR_HEIGHT);
    }

    public void setHeight(String height) {
	setAttribute(HTMLConstants.ATTR_HEIGHT, height);
    }

    /**
     *  Suppress word wrapping. See the  nowrap attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public boolean getNoWrap() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_NO_WRAP);
    }

    public void setNoWrap(boolean noWrap) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_NO_WRAP, noWrap);
    }

    /**
     *  Number of rows spanned by cell. See the  rowspan attribute definition 
     * in HTML 4.0.
     */
    public int getRowSpan() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_ROW_SPAN);
    }

    public void setRowSpan(int rowSpan) {
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_ROW_SPAN, rowSpan);
    }

    /**
     *  Scope covered by header cells. See the  scope attribute definition in 
     * HTML 4.0.
     */
    public String getScope() {
	return getAttribute(HTMLConstants.ATTR_SCOPE);
    }

    public void setScope(String scope) {
	setAttribute(HTMLConstants.ATTR_SCOPE, scope);
    }

    /**
     *  Vertical alignment of data in cell. See the  valign attribute 
     * definition in HTML 4.0.
     */
    public String getVAlign() {
	return getAttribute(HTMLConstants.ATTR_VALIGN);
    }

    public void setVAlign(String vAlign) {
	setAttribute(HTMLConstants.ATTR_VALIGN, vAlign);
    }

    /**
     *  Cell width. See the  width attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }

}

