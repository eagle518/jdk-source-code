/*
 * @(#)HTMLTableSectionElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  The <code>THEAD</code> , <code>TFOOT</code> , and <code>TBODY</code> 
 * elements. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTableSectionElement extends HTMLElement 
		    implements org.w3c.dom.html.HTMLTableSectionElement {
    public HTMLTableSectionElement(DOMObject obj, 
				   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Horizontal alignment of data in cells. See the <code>align</code> 
     * attribute for HTMLTheadElement for details. 
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
     *  Vertical alignment of data in cells. See the <code>valign</code> 
     * attribute for HTMLTheadElement for details. 
     */
    public String getVAlign() {
	return getAttribute(HTMLConstants.ATTR_VALIGN);
    }

    public void setVAlign(String vAlign) {
	setAttribute(HTMLConstants.ATTR_VALIGN, vAlign);
    }

    /**
     *  The collection of rows in this table section. 
     */
    public org.w3c.dom.html.HTMLCollection getRows() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.ATTR_ROWS),
                                                     (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    /**
     *  Insert a row into this section. The new row is inserted immediately 
     * before the current <code>index</code> th row in this section. If 
     * <code>index</code> is equal to the number of rows in this section, the 
     * new row is appended.
     * @param index  The row number where to insert a new row. This index 
     *   starts from 0 and is relative only to the rows contained inside this 
     *   section, not all the rows in the table.
     * @return  The newly created row.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified index is greater than the 
     *   number of rows of if the index is neagative.
     */
    public org.w3c.dom.html.HTMLElement insertRow(int index)
	throws DOMException {
	Object result = obj.call(HTMLConstants.FUNC_INSERT_ROW, new Object[]{new Integer(index)});
	if(result != null && result instanceof DOMObject)
	    return DOMObjectFactory.createHTMLElement((DOMObject)result,
		(org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	return null;
    }

    /**
     *  Delete a row from this section.
     * @param index  The index of the row to be deleted. This index starts 
     *   from 0 and is relative only to the rows contained inside this 
     *   section, not all the rows in the table.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified index is greater than or 
     *   equal to the number of rows or if the index is negative.
     */
    public void deleteRow(int index)
	throws DOMException {
	obj.call(HTMLConstants.FUNC_DELETE_ROW, new Object[]{new Integer(index)});
    }

}

