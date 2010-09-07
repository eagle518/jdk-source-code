/*
 * @(#)HTMLTableElement.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;
import sun.plugin.dom.exception.PluginNotSupportedException;

/**
 *  The create* and delete* methods on the table allow authors to construct 
 * and modify tables. HTML 4.0 specifies that only one of each of the 
 * <code>CAPTION</code> , <code>THEAD</code> , and <code>TFOOT</code> 
 * elements may exist in a table. Therefore, if one exists, and the 
 * createTHead() or createTFoot() method is called, the method returns the 
 * existing THead or TFoot element. See the  TABLE element definition in HTML 
 * 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLTableElement extends HTMLElement 
				    implements org.w3c.dom.html.HTMLTableElement {

    public HTMLTableElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Returns the table's <code>CAPTION</code> , or void if none exists. 
     */
    public org.w3c.dom.html.HTMLTableCaptionElement getCaption() {
	Object result = obj.getMember(HTMLConstants.ATTR_CAPTION);
	if(result != null && result instanceof DOMObject) {
		Object ret = DOMObjectFactory.createHTMLElement((DOMObject)result, 
			    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
		if(ret != null && ret instanceof org.w3c.dom.html.HTMLTableCaptionElement) {
		    return (org.w3c.dom.html.HTMLTableCaptionElement)ret;
		}
	}
	return null;
    }

    public void setCaption(org.w3c.dom.html.HTMLTableCaptionElement caption) {
	if(caption != null && caption instanceof sun.plugin.dom.html.HTMLElement) {
	    DOMObject domObj = ((sun.plugin.dom.html.HTMLElement)caption).getDOMObject();
	    obj.setMember(HTMLConstants.ATTR_CAPTION, domObj);
	    return;
	}

	throw new PluginNotSupportedException("HTMLTableElement.setCaption() is not supported.");
    }

    /**
     *  Returns the table's <code>THEAD</code> , or <code>null</code> if none 
     * exists. 
     */
    public org.w3c.dom.html.HTMLTableSectionElement getTHead() {
	Object result = obj.getMember(HTMLConstants.ATTR_THEAD);
	if(result != null && result instanceof DOMObject) {
		Object ret = DOMObjectFactory.createHTMLElement((DOMObject)result, 
		    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
		if(ret != null && ret instanceof org.w3c.dom.html.HTMLTableSectionElement) {
		    return (org.w3c.dom.html.HTMLTableSectionElement)ret;
		}
	}
	return null;
    }

    public void setTHead(org.w3c.dom.html.HTMLTableSectionElement tHead) {
	if(tHead != null && tHead instanceof sun.plugin.dom.html.HTMLElement) {
	    DOMObject domObj = ((sun.plugin.dom.html.HTMLElement)tHead).getDOMObject();
	    obj.setMember(HTMLConstants.ATTR_THEAD, domObj); 
	    return;
	}

	throw new PluginNotSupportedException("HTMLTableElement.setTHread() is not supported.");
    }

    /**
     *  Returns the table's <code>TFOOT</code> , or <code>null</code> if none 
     * exists. 
     */
    public org.w3c.dom.html.HTMLTableSectionElement getTFoot() {
	Object result = obj.getMember(HTMLConstants.ATTR_TFOOT);
	if(result != null && result instanceof DOMObject) {
		Object ret = DOMObjectFactory.createHTMLElement((DOMObject)result, 
		    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
		if(ret != null && ret instanceof org.w3c.dom.html.HTMLTableSectionElement) {
		    return (org.w3c.dom.html.HTMLTableSectionElement)ret;
		}
	}
	return null;
    }

    public void setTFoot(org.w3c.dom.html.HTMLTableSectionElement tFoot) {
	if(tFoot != null && tFoot instanceof sun.plugin.dom.html.HTMLElement) {
	    DOMObject domObj = ((sun.plugin.dom.html.HTMLElement)tFoot).getDOMObject();
	    obj.setMember(HTMLConstants.ATTR_TFOOT, domObj);
	    return;
	}

	throw new PluginNotSupportedException("HTMLTableElement.setTFoot() is not supported.");
    }

    /**
     *  Returns a collection of all the rows in the table, including all in 
     * <code>THEAD</code> , <code>TFOOT</code> , all <code>TBODY</code> 
     * elements. 
     */
    public org.w3c.dom.html.HTMLCollection getRows() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.ATTR_ROWS),
                                                     (org.w3c.dom.html.HTMLDocument) getOwnerDocument());
    }

    /**
     *  Returns a collection of the defined table bodies. 
     */
    public org.w3c.dom.html.HTMLCollection getTBodies() {
	return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.ATTR_TBODIES),
                                                     (org.w3c.dom.html.HTMLDocument) getOwnerDocument());
    }

    /**
     *  Specifies the table's position with respect to the rest of the 
     * document. See the  align attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
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
     *  The width of the border around the table. See the  border attribute 
     * definition in HTML 4.0.
     */
    public String getBorder() {
	return getAttribute(HTMLConstants.ATTR_BORDER);
    }

    public void setBorder(String border) {
	setAttribute(HTMLConstants.ATTR_BORDER, border);
    }

    /**
     *  Specifies the horizontal and vertical space between cell content and 
     * cell borders. See the  cellpadding attribute definition in HTML 4.0.
     */
    public String getCellPadding() {
	return getAttribute(HTMLConstants.ATTR_CELL_PADDING);
    }

    public void setCellPadding(String cellPadding) {
	setAttribute(HTMLConstants.ATTR_CELL_PADDING, cellPadding);
    }

    /**
     *  Specifies the horizontal and vertical separation between cells. See 
     * the  cellspacing attribute definition in HTML 4.0.
     */
    public String getCellSpacing() {
	return getAttribute(HTMLConstants.ATTR_CELL_SPACING);
    }

    public void setCellSpacing(String cellSpacing) {
	setAttribute(HTMLConstants.ATTR_CELL_SPACING, cellSpacing);
    }

    /**
     *  Specifies which external table borders to render. See the  frame 
     * attribute definition in HTML 4.0.
     */
    public String getFrame() {
	return getAttribute(HTMLConstants.ATTR_FRAME);
    }

    public void setFrame(String frame) {
	setAttribute(HTMLConstants.ATTR_FRAME, frame);
    }

    /**
     *  Specifies which internal table borders to render. See the  rules 
     * attribute definition in HTML 4.0.
     */
    public String getRules() {
	return getAttribute(HTMLConstants.ATTR_RULES);
    }

    public void setRules(String rules) {
	setAttribute(HTMLConstants.ATTR_RULES, rules);
    }

    /**
     *  Description about the purpose or structure of a table. See the  
     * summary attribute definition in HTML 4.0.
     */
    public String getSummary() {
	return getAttribute(HTMLConstants.ATTR_SUMMARY);
    }

    public void setSummary(String summary) {
	setAttribute(HTMLConstants.ATTR_SUMMARY, summary);
    }

    /**
     *  Specifies the desired table width. See the  width attribute definition 
     * in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }

    /**
     *  Create a table header row or return an existing one.
     * @return  A new table header element (<code>THEAD</code> ).
     */
    public org.w3c.dom.html.HTMLElement createTHead() {
	Object result = obj.call(HTMLConstants.FUNC_CREATE_THEAD, null);
	if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createHTMLElement((DOMObject)result, 
			    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	}
	return null;
    }

    /**
     *  Delete the header from the table, if one exists.
     */
    public void deleteTHead() {
	obj.call(HTMLConstants.FUNC_DELETE_THEAD, null);
    }

    /**
     *  Create a table footer row or return an existing one.
     * @return  A footer element (<code>TFOOT</code> ).
     */
    public org.w3c.dom.html.HTMLElement createTFoot() {
	Object result = obj.call(HTMLConstants.FUNC_CREATE_TFOOT, null);
	if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createHTMLElement((DOMObject)result, 
			    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	}
	return null;
    }

    /**
     *  Delete the footer from the table, if one exists.
     */
    public void deleteTFoot() {
	obj.call(HTMLConstants.FUNC_DELETE_TFOOT, null);
    }

    /**
     *  Create a new table caption object or return an existing one.
     * @return  A <code>CAPTION</code> element.
     */
    public org.w3c.dom.html.HTMLElement createCaption() {
	Object result = obj.call(HTMLConstants.FUNC_CREATE_CAPTION, null);
	if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createHTMLElement((DOMObject)result, 
			    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	}
	return null;
    }

    /**
     *  Delete the table caption, if one exists.
     */
    public void deleteCaption() {
	obj.call(HTMLConstants.FUNC_DELETE_CAPTION, null);
    }

    /**
     *  Insert a new empty row in the table. The new row is inserted 
     * immediately before and in the same section as the current 
     * <code>index</code> th row in the table. If <code>index</code> is equal 
     * to the number of rows, the new row is appended. In addition, when the 
     * table is empty the row is inserted into a <code>TBODY</code> which is 
     * created and inserted into the table. Note. A table row cannot be empty 
     * according to HTML 4.0 Recommendation.
     * @param index  The row number where to insert a new row. This index 
     *   starts from 0 and is relative to all the rows contained inside the 
     *   table, regardless of section parentage.
     * @return  The newly created row.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified index is greater than the 
     *   number of rows or if the index is negative.
     */
    public org.w3c.dom.html.HTMLElement insertRow(int index)
                                 throws DOMException {

	Object result = obj.call(HTMLConstants.FUNC_INSERT_ROW, new Object[]{ new Integer(index) });
	if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createHTMLElement((DOMObject)result, 
			    (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
	}
	return null;
    }

    /**
     *  Delete a table row.
     * @param index  The index of the row to be deleted. This index starts 
     *   from 0 and is relative to all the rows contained inside the table, 
     *   regardless of section parentage.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified index is greater than or 
     *   equal to the number of rows or if the index is negative.
     */
    public void deleteRow(int index)
	  throws DOMException {
	obj.call(HTMLConstants.FUNC_DELETE_ROW, new Object[]{ new Integer(index) });
    }
}
