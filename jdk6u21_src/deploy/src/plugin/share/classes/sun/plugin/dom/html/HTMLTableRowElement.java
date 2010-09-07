/*
 * @(#)HTMLTableRowElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  A row in a table. See the  TR element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTableRowElement extends HTMLElement 
				       implements org.w3c.dom.html.HTMLTableRowElement {

    public HTMLTableRowElement(DOMObject obj, 
			       org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The index of this row, relative to the entire table, starting from 0. 
     * This is in document tree order and not display order. The 
     * <code>rowIndex</code> does not take into account sections (
     * <code>THEAD</code> , <code>TFOOT</code> , or <code>TBODY</code> ) 
     * within the table.
     */
    public int getRowIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_ROW_INDEX);
    }

    /**
     *  The index of this row, relative to the current section (
     * <code>THEAD</code> , <code>TFOOT</code> , or <code>TBODY</code> ), 
     * starting from 0.
     */
    public int getSectionRowIndex() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_SECTION_ROW_INDEX);
    }

    /**
     *  The collection of cells in this row. 
     */
    public org.w3c.dom.html.HTMLCollection getCells() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.ATTR_CELLS),
                                                     (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    /**
     *  Horizontal alignment of data within cells of this row. See the  align 
     * attribute definition in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }

    /**
     *  Background color for rows. See the  bgcolor attribute definition in 
     * HTML 4.0. This attribute is deprecated in HTML 4.0.
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
     *  Vertical alignment of data within cells of this row. See the  valign 
     * attribute definition in HTML 4.0.
     */
    public String getVAlign() {
	return getAttribute(HTMLConstants.ATTR_VALIGN);
    }

    public void setVAlign(String vAlign) {
	setAttribute(HTMLConstants.ATTR_VALIGN, vAlign);
    }

    /**
     *  Insert an empty <code>TD</code> cell into this row. If 
     * <code>index</code> is equal to the number of cells, the new cell is 
     * appended
     * @param index  The place to insert the cell, starting from 0.
     * @return  The newly created cell.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>index</code> is 
     *   greater than the number of cells or if the index is negative.
     */
    public org.w3c.dom.html.HTMLElement insertCell(int index)
	throws DOMException {
	Object result = obj.call(HTMLConstants.FUNC_INSERT_CELL, new Object[]{new Integer(index)});
	if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createHTMLElement((DOMObject)result, 
		    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	}
	return null;
    }

    /**
     *  Delete a cell from the current row.
     * @param index  The index of the cell to delete, starting from 0.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>index</code> is 
     *   greater than or equal to the number of cells or if the index is 
     *   negative.
     */
    public void deleteCell(int index)
	throws DOMException {
	obj.call(HTMLConstants.FUNC_DELETE_CELL, new Object[]{new Integer(index)});
    }
}

