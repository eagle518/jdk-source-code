/*
 * @(#)CSSStyleDeclaration.java	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 /*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

package sun.plugin.dom.css;

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;
import sun.plugin.dom.exception.PluginNotSupportedException;

/**
 *  The <code>CSSStyleDeclaration</code> interface represents a single CSS 
 * declaration block. This interface may be used to determine the style 
 * properties currently set in a block or to set style properties explicitly 
 * within the block. 
 * <p> While an implementation may not recognize all CSS properties within a 
 * CSS declaration block, it is expected to provide access to all specified 
 * properties in the style sheet through the <code>CSSStyleDeclaration</code>
 *  interface. Furthermore, implementations that support a specific level of 
 * CSS should correctly handle CSS shorthand properties for that level. For 
 * a further discussion of shorthand properties, see the 
 * <code>CSS2Properties</code> interface. 
 * <p> This interface is also used to provide a read-only access to the 
 * computed values of an element. See also the <code>ViewCSS</code> 
 * interface.  The CSS Object Model doesn't provide an access to the 
 * specified or actual values of the CSS cascade. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSStyleDeclaration implements org.w3c.dom.css.CSSStyleDeclaration,
					    org.w3c.dom.css.CSS2Properties {
    // Underlying DOMObject
    protected DOMObject obj;

    // Owner document
    private org.w3c.dom.Document document;

    /**
     * Construct a CSSRuleList object.
     */
    public CSSStyleDeclaration(DOMObject obj,
			       org.w3c.dom.Document doc) {
	this.obj = obj;
	this.document = doc;
    }

    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSSStyleDeclaration
    //------------------------------------------------------------

    /**
     *  The parsable textual representation of the declaration block 
     * (excluding the surrounding curly braces). Setting this attribute will 
     * result in the parsing of the new value and resetting of all the 
     * properties in the declaration block including the removal or addition 
     * of properties. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified CSS string value has a syntax 
     *   error and is unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is 
     *   readonly or a property is readonly.
     */
    public String getCssText() {
	return getPropertyValue(CSSConstants.ATTR_CSS_TEXT);
    }

    public void setCssText(String cssText)
                       throws DOMException {
	setProperty(CSSConstants.ATTR_CSS_TEXT, cssText, null);
    }

    /**
     *  Used to retrieve the value of a CSS property if it has been explicitly 
     * set within this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns the empty string if the 
     *   property has not been set. 
     */
    public String getPropertyValue(String propertyName) {
	return DOMObjectHelper.getStringMemberNoEx(obj, propertyName);
    }

    /**
     * Used to retrieve the object representation of the value of a CSS 
     * property if it has been explicitly set within this declaration block. 
     * This method returns <code>null</code> if the property is a shorthand 
     * property. Shorthand property values can only be accessed and modified 
     * as strings, using the <code>getPropertyValue</code> and 
     * <code>setProperty</code> methods. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns <code>null</code> if the 
     *   property has not been set. 
     */
    public org.w3c.dom.css.CSSValue getPropertyCSSValue(String propertyName) {
        return DOMObjectFactory.createCSSValue(
            obj.call(CSSConstants.FUNC_GET_PROPERTY_VALUE, new Object[] { propertyName }),
            document);
    }

    /**
     *  Used to remove a CSS property if it has been explicitly set within 
     * this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  Returns the value of the property if it has been explicitly 
     *   set for this declaration block. Returns the empty string if the 
     *   property has not been set or the property name does not correspond 
     *   to a known CSS property. 
     * @exception DOMException
     *   NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is readonly 
     *   or the property is readonly.
     */
    public String removeProperty(String propertyName)
                                 throws DOMException {
        return DOMObjectHelper.callStringMethod(obj,
                                                CSSConstants.FUNC_REMOVE_PROPERTY,
                                                new Object[] { propertyName });
    }

    /**
     *  Used to retrieve the priority of a CSS property (e.g. the 
     * <code>"important"</code> qualifier) if the property has been 
     * explicitly set in this declaration block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @return  A string representing the priority (e.g. 
     *   <code>"important"</code>) if one exists. The empty string if none 
     *   exists. 
     */
    public String getPropertyPriority(String propertyName) {
        return DOMObjectHelper.callStringMethod(obj,
                                                CSSConstants.FUNC_GET_PROPERTY_PRIORITY,
                                                new Object[] { propertyName });
    }

    /**
     *  Used to set a property value and priority within this declaration 
     * block. 
     * @param propertyName The name of the CSS property. See the CSS property 
     *   index. 
     * @param value The new value of the property. 
     * @param priority The new priority of the property (e.g. 
     *   <code>"important"</code>).  
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this declaration is 
     *   readonly or the property is readonly.
     */
    public void setProperty(String propertyName, 
                            String value, 
                            String priority)
                            throws DOMException {
	DOMObjectHelper.setStringMember(obj, propertyName, value);
    }

    /**
     *  The number of properties that have been explicitly set in this 
     * declaration block. The range of valid indices is 0 to length-1 
     * inclusive. 
     */
    public int getLength() {
        return ((Number) obj.getMember(CSSConstants.ATTR_LENGTH)).intValue();
    }

    /**
     *  Used to retrieve the properties that have been explicitly set in this 
     * declaration block. The order of the properties retrieved using this 
     * method does not have to be the order in which they were set. This 
     * method can be used to iterate over all properties in this declaration 
     * block. 
     * @param index Index of the property name to retrieve. 
     * @return  The name of the property at this ordinal position. The empty 
     *   string if no property exists at this position. 
     */
    public String item(int index) {
        return DOMObjectHelper.callStringMethod(obj,
                                                CSSConstants.FUNC_ITEM,
                                                new Object[] { new Integer(index) });
    }	

    /**
     *  The CSS rule that contains this declaration block or <code>null</code> 
     * if this <code>CSSStyleDeclaration</code> is not attached to a 
     * <code>CSSRule</code>. 
     */
    public org.w3c.dom.css.CSSRule getParentRule() {
        return DOMObjectFactory.createCSSRule((DOMObject) obj.getMember(CSSConstants.ATTR_PARENT_RULE),
                                              document);
    }
    
    
    //------------------------------------------------------------
    // Method from org.w3c.dom.css.CSS2Properties
    //------------------------------------------------------------

    /**
     *  See the azimuth property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getAzimuth() {
	return getPropertyValue(CSSConstants.ATTR_AZIMUTH);
    }

    public void setAzimuth(String azimuth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_AZIMUTH, azimuth, null);
    }


    /**
     *  See the background property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackground() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND);
    }

    public void setBackground(String background)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND, background, null);
    }


    /**
     *  See the background-attachment property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundAttachment() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND_ATTACHMENT);
    }

    public void setBackgroundAttachment(String backgroundAttachment)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND_ATTACHMENT, backgroundAttachment, null);
    }


    /**
     *  See the background-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundColor() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND_COLOR);
    }

    public void setBackgroundColor(String backgroundColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND_COLOR, backgroundColor, null);
    }


    /**
     *  See the background-image property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundImage() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND_IMAGE);
    }

    public void setBackgroundImage(String backgroundImage)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND_IMAGE, backgroundImage, null);
    }


    /**
     *  See the background-position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundPosition() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND_POSITION);
    }

    public void setBackgroundPosition(String backgroundPosition)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND_POSITION, backgroundPosition, null);
    }


    /**
     *  See the background-repeat property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBackgroundRepeat() {
	return getPropertyValue(CSSConstants.ATTR_BACKGROUND_REPEAT);
    }

    public void setBackgroundRepeat(String backgroundRepeat)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BACKGROUND_REPEAT, backgroundRepeat, null);
    }


    /**
     *  See the border property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorder() {
	return getPropertyValue(CSSConstants.ATTR_BORDER);
    }

    public void setBorder(String border)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER, border, null);
    }


    /**
     *  See the border-collapse property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderCollapse() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_COLLAPSE);
    }

    public void setBorderCollapse(String borderCollapse)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_COLLAPSE, borderCollapse, null);
    }


    /**
     *  See the border-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderColor() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_COLOR);
    }

    public void setBorderColor(String borderColor)
	throws DOMException{
	setProperty(CSSConstants.ATTR_BORDER_COLOR, borderColor, null);
    }


    /**
     *  See the border-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderSpacing() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_SPACING);
    }

    public void setBorderSpacing(String borderSpacing)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_SPACING, borderSpacing, null);
    }


    /**
     *  See the border-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderStyle() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_STYLE);
    }

    public void setBorderStyle(String borderStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_STYLE, borderStyle, null);
    }


    /**
     *  See the border-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTop() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_TOP);
    }

    public void setBorderTop(String borderTop)
	throws DOMException{
	setProperty(CSSConstants.ATTR_BORDER_TOP, borderTop, null);
    }

    /**
     *  See the border-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRight(){
	return getPropertyValue(CSSConstants.ATTR_BORDER_RIGHT);
    }

    public void setBorderRight(String borderRight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_RIGHT, borderRight, null);
    }

    /**
     *  See the border-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottom() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_BOTTOM);
    }

    public void setBorderBottom(String borderBottom)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_BOTTOM, borderBottom, null);
    }

    /**
     *  See the border-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeft() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_LEFT);
    }

    public void setBorderLeft(String borderLeft)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_LEFT, borderLeft, null);
    }

    /**
     *  See the border-top-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopColor() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_TOP_COLOR);
    }

    public void setBorderTopColor(String borderTopColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_TOP_COLOR, borderTopColor, null);
    }

    /**
     *  See the border-right-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightColor() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_RIGHT_COLOR);
    }

    public void setBorderRightColor(String borderRightColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_RIGHT_COLOR, borderRightColor, null);
    }

    /**
     *  See the border-bottom-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomColor() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_BOTTOM_COLOR);
    }

    public void setBorderBottomColor(String borderBottomColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_BOTTOM_COLOR, borderBottomColor, null);
    }

    /**
     *  See the border-left-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftColor()
    {
	return getPropertyValue(CSSConstants.ATTR_BORDER_LEFT_COLOR);
    }

    public void setBorderLeftColor(String borderLeftColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_LEFT_COLOR, borderLeftColor, null);
    }

    /**
     *  See the border-top-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopStyle() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_TOP_STYLE);
    }

    public void setBorderTopStyle(String borderTopStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_TOP_STYLE, borderTopStyle, null);
    }

    /**
     *  See the border-right-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightStyle(){
	return getPropertyValue(CSSConstants.ATTR_BORDER_RIGHT_STYLE);
    }

    public void setBorderRightStyle(String borderRightStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_RIGHT_STYLE, borderRightStyle, null);
    }

    /**
     *  See the border-bottom-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomStyle() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_BOTTOM_STYLE);
    }

    public void setBorderBottomStyle(String borderBottomStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_BOTTOM_STYLE, borderBottomStyle, null);
    }

    /**
     *  See the border-left-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftStyle() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_LEFT_STYLE);
    }

    public void setBorderLeftStyle(String borderLeftStyle)  
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_LEFT_STYLE, borderLeftStyle, null);
    }

    /**
     *  See the border-top-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderTopWidth() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_TOP_WIDTH);
    }

    public void setBorderTopWidth(String borderTopWidth)
	throws DOMException{
	setProperty(CSSConstants.ATTR_BORDER_TOP_WIDTH, borderTopWidth, null);
    }

    /**
     *  See the border-right-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderRightWidth(){
	return getPropertyValue(CSSConstants.ATTR_BORDER_RIGHT_WIDTH);
    }

    public void setBorderRightWidth(String borderRightWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_RIGHT_WIDTH, borderRightWidth, null);
    }

    /**
     *  See the border-bottom-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderBottomWidth() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_BOTTOM_WIDTH);
    }

    public void setBorderBottomWidth(String borderBottomWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_BOTTOM_WIDTH, borderBottomWidth, null);
    }

    /**
     *  See the border-left-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderLeftWidth() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_LEFT_WIDTH);
    }

    public void setBorderLeftWidth(String borderLeftWidth)  
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_LEFT_WIDTH, borderLeftWidth, null);
    }

    /**
     *  See the border-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBorderWidth() {
	return getPropertyValue(CSSConstants.ATTR_BORDER_WIDTH);
    }

    public void setBorderWidth(String borderWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_BORDER_WIDTH, borderWidth, null);
    }

    /**
     *  See the bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getBottom() {
	return getPropertyValue(CSSConstants.ATTR_BOTTOM);
    }

    public void setBottom(String bottom)    
	throws DOMException {
	setProperty(CSSConstants.ATTR_BOTTOM, bottom, null);
    }

    /**
     *  See the caption-side property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCaptionSide() {
	return getPropertyValue(CSSConstants.ATTR_CAPTION_SIDE);
    }

    public void setCaptionSide(String captionSide)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CAPTION_SIDE, captionSide, null);
    }

    /**
     *  See the clear property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getClear() {
	return getPropertyValue(CSSConstants.ATTR_CLEAR);
    }

    public void setClear(String clear)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CLEAR, clear, null);
    }

    /**
     *  See the clip property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getClip() {
	return getPropertyValue(CSSConstants.ATTR_CLIP);
    }

    public void setClip(String clip)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CLIP, clip, null);
    }

    /**
     *  See the color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getColor() {
	return getPropertyValue(CSSConstants.ATTR_COLOR);
    }

    public void setColor(String color)
	throws DOMException {
	setProperty(CSSConstants.ATTR_COLOR, color, null);
    }

    /**
     *  See the content property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getContent() {
	return getPropertyValue(CSSConstants.ATTR_CONTENT);
    }

    public void setContent(String content)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CONTENT, content, null);
    }

    /**
     *  See the counter-increment property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCounterIncrement() {
	return getPropertyValue(CSSConstants.ATTR_COUNTER_INCREMENT);
    }

    public void setCounterIncrement(String counterIncrement)
	throws DOMException {
	setProperty(CSSConstants.ATTR_COUNTER_INCREMENT, counterIncrement, null);
    }

    /**
     *  See the counter-reset property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCounterReset() {
	return getPropertyValue(CSSConstants.ATTR_COUNTER_RESET);
    }

    public void setCounterReset(String counterReset)
	throws DOMException {
	setProperty(CSSConstants.ATTR_COUNTER_RESET, counterReset, null);
    }

    /**
     *  See the cue property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCue() {
	return getPropertyValue(CSSConstants.ATTR_CUE);
    }

    public void setCue(String cue)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CUE, cue, null);
    }

    /**
     *  See the cue-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCueAfter() {
	return getPropertyValue(CSSConstants.ATTR_CUE_AFTER);
    }

    public void setCueAfter(String cueAfter)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CUE_AFTER, cueAfter, null);
    }

    /**
     *  See the cue-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCueBefore() {
	return getPropertyValue(CSSConstants.ATTR_CUE_BEFORE);
    }

    public void setCueBefore(String cueBefore)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CUE_BEFORE, cueBefore, null);
    }

    /**
     *  See the cursor property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCursor() {
	return getPropertyValue(CSSConstants.ATTR_CURSOR);
    }

    public void setCursor(String cursor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CURSOR, cursor, null);
    }

    /**
     *  See the direction property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getDirection() {
	return getPropertyValue(CSSConstants.ATTR_DIRECTION);
    }

    public void setDirection(String direction)
	throws DOMException {
	setProperty(CSSConstants.ATTR_DIRECTION, direction, null);
    }

    /**
     *  See the display property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getDisplay() {
	return getPropertyValue(CSSConstants.ATTR_DISPLAY);
    }

    public void setDisplay(String display)
	throws DOMException {
	setProperty(CSSConstants.ATTR_DISPLAY, display, null);
    }

    /**
     *  See the elevation property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getElevation() {
	return getPropertyValue(CSSConstants.ATTR_ELEVATION);
    }

    public void setElevation(String elevation)
	throws DOMException {
	setProperty(CSSConstants.ATTR_ELEVATION, elevation, null);
    }

    /**
     *  See the empty-cells property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getEmptyCells() {
	return getPropertyValue(CSSConstants.ATTR_EMPTY_CELLS);
    }

    public void setEmptyCells(String emptyCells)
	throws DOMException {
	setProperty(CSSConstants.ATTR_EMPTY_CELLS, emptyCells, null);
    }

    /**
     *  See the float property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getCssFloat() {
	return getPropertyValue(CSSConstants.ATTR_CSS_FLOAT);
    }

    public void setCssFloat(String cssFloat)
	throws DOMException {
	setProperty(CSSConstants.ATTR_CSS_FLOAT, cssFloat, null);
    }

    /**
     *  See the font property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFont() {
	return getPropertyValue(CSSConstants.ATTR_FONT);
    }

    public void setFont(String font)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT, font, null);
    }

    /**
     *  See the font-family property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontFamily() {
	return getPropertyValue(CSSConstants.ATTR_FONT_FAMILY);
    }

    public void setFontFamily(String fontFamily)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_FAMILY, fontFamily, null);
    }

    /**
     *  See the font-size property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontSize() {
	return getPropertyValue(CSSConstants.ATTR_FONT_SIZE);
    }

    public void setFontSize(String fontSize)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_SIZE, fontSize, null);
    }

    /**
     *  See the font-size-adjust property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontSizeAdjust() {
	return getPropertyValue(CSSConstants.ATTR_FONT_SiZE_ADJUST);
    }

    public void setFontSizeAdjust(String fontSizeAdjust)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_SiZE_ADJUST, fontSizeAdjust, null);
    }

    /**
     *  See the font-stretch property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontStretch() {
	return getPropertyValue(CSSConstants.ATTR_FONT_STRETCH);
    }

    public void setFontStretch(String fontStretch)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_STRETCH, fontStretch, null);
    }

    /**
     *  See the font-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontStyle() {
	return getPropertyValue(CSSConstants.ATTR_FONT_STYLE);
    }

    public void setFontStyle(String fontStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_STYLE, fontStyle, null);
    }

    /**
     *  See the font-variant property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontVariant() {
	return getPropertyValue(CSSConstants.ATTR_FONT_VARIANT);
    }

    public void setFontVariant(String fontVariant)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_VARIANT, fontVariant, null);
    }

    /**
     *  See the font-weight property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getFontWeight() {
	return getPropertyValue(CSSConstants.ATTR_FONT_WEIGHT);
    }

    public void setFontWeight(String fontWeight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_FONT_WEIGHT, fontWeight, null);
    }

    /**
     *  See the height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getHeight() {
	return getPropertyValue(CSSConstants.ATTR_HEIGHT);
    }

    public void setHeight(String height)
	throws DOMException {
	setProperty(CSSConstants.ATTR_HEIGHT, height, null);
    }

    /**
     *  See the left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLeft() {
	return getPropertyValue(CSSConstants.ATTR_LEFT);
    }

    public void setLeft(String left)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LEFT, left, null);
    }

    /**
     *  See the letter-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLetterSpacing() {
	return getPropertyValue(CSSConstants.ATTR_LETTER_SPACING);
    }

    public void setLetterSpacing(String letterSpacing)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LETTER_SPACING, letterSpacing, null);
    }

    /**
     *  See the line-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getLineHeight() {
	return getPropertyValue(CSSConstants.ATTR_LINE_HEIGHT);
    }

    public void setLineHeight(String lineHeight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LINE_HEIGHT, lineHeight, null);
    }

    /**
     *  See the list-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyle() {
	return getPropertyValue(CSSConstants.ATTR_LIST_STYLE);
    }

    public void setListStyle(String listStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LIST_STYLE, listStyle, null);
    }

    /**
     *  See the list-style-image property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyleImage(){
	return getPropertyValue(CSSConstants.ATTR_LIST_STYLE_IMAGE);
    }

    public void setListStyleImage(String listStyleImage)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LIST_STYLE_IMAGE, listStyleImage, null);
    }

    /**
     *  See the list-style-position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStylePosition() {
	return getPropertyValue(CSSConstants.ATTR_LIST_STYLE_POSITION);
    }

    public void setListStylePosition(String listStylePosition)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LIST_STYLE_POSITION, listStylePosition, null);
    }

    /**
     *  See the list-style-type property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getListStyleType() {
	return getPropertyValue(CSSConstants.ATTR_LIST_STYLE_TYPE);
    }

    public void setListStyleType(String listStyleType)
	throws DOMException {
	setProperty(CSSConstants.ATTR_LIST_STYLE_TYPE, listStyleType, null);
    }

    /**
     *  See the margin property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMargin() {
	return getPropertyValue(CSSConstants.ATTR_MARGIN);
    }

    public void setMargin(String margin)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARGIN, margin, null);
    }

    /**
     *  See the margin-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginTop() {
	return getPropertyValue(CSSConstants.ATTR_MARGIN_TOP);
    }

    public void setMarginTop(String marginTop)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARGIN_TOP, marginTop, null);
    }

    /**
     *  See the margin-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginRight() {
	return getPropertyValue(CSSConstants.ATTR_MARGIN_RIGHT);
    }

    public void setMarginRight(String marginRight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARGIN_RIGHT, marginRight, null);
    }

    /**
     *  See the margin-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginBottom() {
	return getPropertyValue(CSSConstants.ATTR_MARGIN_BOTTOM);
    }

    public void setMarginBottom(String marginBottom)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARGIN_BOTTOM, marginBottom, null);
    }

    /**
     *  See the margin-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarginLeft() {
	return getPropertyValue(CSSConstants.ATTR_MARGIN_LEFT);
    }

    public void setMarginLeft(String marginLeft)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARGIN_LEFT, marginLeft, null);
    }

    /**
     *  See the marker-offset property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarkerOffset() {
	return getPropertyValue(CSSConstants.ATTR_MARKER_OFFSET);
    }

    public void setMarkerOffset(String markerOffset)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARKER_OFFSET, markerOffset, null);
    }

    /**
     *  See the marks property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMarks() {
	return getPropertyValue(CSSConstants.ATTR_MARKS);
    }

    public void setMarks(String marks)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MARKS, marks, null);
    }

    /**
     *  See the max-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMaxHeight() {
	return getPropertyValue(CSSConstants.ATTR_MAX_HEIGHT);
    }

    public void setMaxHeight(String maxHeight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MAX_HEIGHT, maxHeight, null);
    }

    /**
     *  See the max-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMaxWidth() {
	return getPropertyValue(CSSConstants.ATTR_MAX_WIDTH);
    }

    public void setMaxWidth(String maxWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MAX_WIDTH, maxWidth, null);
    }

    /**
     *  See the min-height property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMinHeight() {
	return getPropertyValue(CSSConstants.ATTR_MIN_HEIGHT);
    }

    public void setMinHeight(String minHeight)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MIN_HEIGHT, minHeight, null);
    }

    /**
     *  See the min-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getMinWidth() {
	return getPropertyValue(CSSConstants.ATTR_MIN_WIDTH);
    }

    public void setMinWidth(String minWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_MIN_WIDTH, minWidth, null);
    }

    /**
     *  See the orphans property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOrphans() {
	return getPropertyValue(CSSConstants.ATTR_ORPHANS);
    }

    public void setOrphans(String orphans)
	throws DOMException {
	setProperty(CSSConstants.ATTR_ORPHANS, orphans, null);
    }

    /**
     *  See the outline property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutline() {
	return getPropertyValue(CSSConstants.ATTR_OUTLINE);
    }

    public void setOutline(String outline)
	throws DOMException {
	setProperty(CSSConstants.ATTR_OUTLINE, outline, null);
    }

    /**
     *  See the outline-color property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineColor() {
	return getPropertyValue(CSSConstants.ATTR_OUTLINE_COLOR);
    }

    public void setOutlineColor(String outlineColor)
	throws DOMException {
	setProperty(CSSConstants.ATTR_OUTLINE_COLOR, outlineColor, null);
    }

    /**
     *  See the outline-style property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineStyle() {
	return getPropertyValue(CSSConstants.ATTR_OUTLINE_STYLE);
    }

    public void setOutlineStyle(String outlineStyle)
	throws DOMException {
	setProperty(CSSConstants.ATTR_OUTLINE_STYLE, outlineStyle, null);
    }

    /**
     *  See the outline-width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOutlineWidth() {
	return getPropertyValue(CSSConstants.ATTR_OUTLINE_WIDTH);
    }

    public void setOutlineWidth(String outlineWidth)
	throws DOMException {
	setProperty(CSSConstants.ATTR_OUTLINE_WIDTH, outlineWidth, null);
    }

    /**
     *  See the overflow property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getOverflow() {
	return getPropertyValue(CSSConstants.ATTR_OVERFLOW);
    }

    public void setOverflow(String overflow)
	throws DOMException {
	setProperty(CSSConstants.ATTR_OVERFLOW, overflow, null);
    }

    /**
     *  See the padding property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPadding() {
	return getPropertyValue(CSSConstants.ATTR_PADDING);
    }

    public void setPadding(String padding) 
	throws DOMException {
	setProperty(CSSConstants.ATTR_PADDING, padding, null);
    }

    /**
     *  See the padding-top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingTop() {
	return getPropertyValue(CSSConstants.ATTR_PADDING_TOP);
    }

    public void setPaddingTop(String paddingTop)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PADDING_TOP, paddingTop, null);
    }

    /**
     *  See the padding-right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingRight(){
	return getPropertyValue(CSSConstants.ATTR_PADDING_RIGHT);
    }

    public void setPaddingRight(String paddingRight) 
	throws DOMException {
	setProperty(CSSConstants.ATTR_PADDING_RIGHT, paddingRight, null);
    }

    /**
     *  See the padding-bottom property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingBottom() {
	return getPropertyValue(CSSConstants.ATTR_PADDING_BOTTOM);
    }

    public void setPaddingBottom(String paddingBottom)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PADDING_BOTTOM, paddingBottom, null);
    }

    /**
     *  See the padding-left property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPaddingLeft() {
	return getPropertyValue(CSSConstants.ATTR_PADDING_LEFT);
    }

    public void setPaddingLeft(String paddingLeft)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PADDING_LEFT, paddingLeft, null);
    }

    /**
     *  See the page property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPage() {
	return getPropertyValue(CSSConstants.ATTR_PAGE);
    }

    public void setPage(String page)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAGE, page, null);
    }

    /**
     *  See the page-break-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakAfter() {
	return getPropertyValue(CSSConstants.ATTR_PAGE_BREAK_AFTER);
    }

    public void setPageBreakAfter(String pageBreakAfter)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAGE_BREAK_AFTER, pageBreakAfter, null);
    }

    /**
     *  See the page-break-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakBefore() {
	return getPropertyValue(CSSConstants.ATTR_PAGE_BREAK_BEFORE);
    }

    public void setPageBreakBefore(String pageBreakBefore)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAGE_BREAK_BEFORE, pageBreakBefore, null);
    }

    /**
     *  See the page-break-inside property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPageBreakInside() {
	return getPropertyValue(CSSConstants.ATTR_PAGE_BREAK_INSIDE);
    }

    public void setPageBreakInside(String pageBreakInside)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAGE_BREAK_INSIDE, pageBreakInside, null);
    }

    /**
     *  See the pause property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPause() {
	return getPropertyValue(CSSConstants.ATTR_PAUSE);
    }

    public void setPause(String pause)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAUSE, pause, null);
    }

    /**
     *  See the pause-after property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPauseAfter() {
	return getPropertyValue(CSSConstants.ATTR_PAUSE_AFTER);
    }

    public void setPauseAfter(String pauseAfter)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAUSE_AFTER, pauseAfter, null);
    }

    /**
     *  See the pause-before property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPauseBefore() {
	return getPropertyValue(CSSConstants.ATTR_PAUSE_BEFORE);
    }

    public void setPauseBefore(String pauseBefore)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PAUSE_BEFORE, pauseBefore, null);
    }

    /**
     *  See the pitch property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPitch() {
	return getPropertyValue(CSSConstants.ATTR_PITCH);
    }

    public void setPitch(String pitch) 
	throws DOMException {
	setProperty(CSSConstants.ATTR_PITCH, pitch, null);
    }

    /**
     *  See the pitch-range property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPitchRange() {
	return getPropertyValue(CSSConstants.ATTR_PITCH_RANGE);
    }

    public void setPitchRange(String pitchRange)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PITCH_RANGE, pitchRange, null);
    }

    /**
     *  See the play-during property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPlayDuring() {
	return getPropertyValue(CSSConstants.ATTR_PLAY_DURING);
    }

    public void setPlayDuring(String playDuring)
	throws DOMException {
	setProperty(CSSConstants.ATTR_PLAY_DURING, playDuring, null);
    }

    /**
     *  See the position property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getPosition() {
	return getPropertyValue(CSSConstants.ATTR_POSITION);
    }

    public void setPosition(String position)
	throws DOMException {
	setProperty(CSSConstants.ATTR_POSITION, position, null);
    }

    /**
     *  See the quotes property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getQuotes() {
	return getPropertyValue(CSSConstants.ATTR_QUOTES);
    }

    public void setQuotes(String quotes)
	throws DOMException {
	setProperty(CSSConstants.ATTR_QUOTES, quotes, null);
    }

    /**
     *  See the richness property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getRichness() {
	return getPropertyValue(CSSConstants.ATTR_RICHNESS);
    }

    public void setRichness(String richness)
	throws DOMException {
	setProperty(CSSConstants.ATTR_RICHNESS, richness, null);
    }

    /**
     *  See the right property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getRight() {
	return getPropertyValue(CSSConstants.ATTR_RIGHT);
    }

    public void setRight(String right)
	throws DOMException {
	setProperty(CSSConstants.ATTR_RIGHT, right, null);
    }

    /**
     *  See the size property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSize() {
	return getPropertyValue(CSSConstants.ATTR_SIZE);
    }

    public void setSize(String size)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SIZE, size, null);
    }

    /**
     *  See the speak property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeak() {
	return getPropertyValue(CSSConstants.ATTR_SPEAK);
    }

    public void setSpeak(String speak)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SPEAK, speak, null);
    }

    /**
     *  See the speak-header property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakHeader() {
	return getPropertyValue(CSSConstants.ATTR_SPEAK_HEADER);
    }

    public void setSpeakHeader(String speakHeader)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SPEAK_HEADER, speakHeader, null);
    }

    /**
     *  See the speak-numeral property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakNumeral() {
	return getPropertyValue(CSSConstants.ATTR_SPEAK_NUMERAL);
    }

    public void setSpeakNumeral(String speakNumeral)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SPEAK_NUMERAL, speakNumeral, null);
    }

    /**
     *  See the speak-punctuation property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeakPunctuation() {
	return getPropertyValue(CSSConstants.ATTR_SPEAK_PUNCTUATION);
    }

    public void setSpeakPunctuation(String speakPunctuation)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SPEAK_PUNCTUATION, speakPunctuation, null);
    }

    /**
     *  See the speech-rate property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getSpeechRate() {
	return getPropertyValue(CSSConstants.ATTR_SPEECH_RATE);
    }

    public void setSpeechRate(String speechRate)
	throws DOMException {
	setProperty(CSSConstants.ATTR_SPEECH_RATE, speechRate, null);
    }

    /**
     *  See the stress property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getStress() {
	return getPropertyValue(CSSConstants.ATTR_STRESS);
    }

    public void setStress(String stress) 
	throws DOMException {
	setProperty(CSSConstants.ATTR_STRESS, stress, null);
    }

    /**
     *  See the table-layout property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTableLayout() {
	return getPropertyValue(CSSConstants.ATTR_TABLE_LAYOUT);
    }

    public void setTableLayout(String tableLayout)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TABLE_LAYOUT, tableLayout, null);
    }

    /**
     *  See the text-align property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextAlign() {
	return getPropertyValue(CSSConstants.ATTR_TEXT_ALIGN);
    }

    public void setTextAlign(String textAlign)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TEXT_ALIGN, textAlign, null);
    }

    /**
     *  See the text-decoration property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextDecoration() {
	return getPropertyValue(CSSConstants.ATTR_TEXT_DECORATION);
    }

    public void setTextDecoration(String textDecoration)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TEXT_DECORATION, textDecoration, null);
    }

    /**
     *  See the text-indent property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextIndent() {
	return getPropertyValue(CSSConstants.ATTR_TEXT_INDENT);
    }

    public void setTextIndent(String textIndent)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TEXT_INDENT, textIndent, null);
    }

    /**
     *  See the text-shadow property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextShadow() {
	return getPropertyValue(CSSConstants.ATTR_TEXT_SHADOW);
    }

    public void setTextShadow(String textShadow)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TEXT_SHADOW, textShadow, null);
    }

    /**
     *  See the text-transform property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTextTransform() {
	return getPropertyValue(CSSConstants.ATTR_TEXT_TRANSFORM);
    }

    public void setTextTransform(String textTransform)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TEXT_TRANSFORM, textTransform, null);
    }

    /**
     *  See the top property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getTop() {
	return getPropertyValue(CSSConstants.ATTR_TOP);
    }

    public void setTop(String top)
	throws DOMException {
	setProperty(CSSConstants.ATTR_TOP, top, null);
    }

    /**
     *  See the unicode-bidi property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getUnicodeBidi() {
	return getPropertyValue(CSSConstants.ATTR_UNICODE_BIDI);
    }

    public void setUnicodeBidi(String unicodeBidi)
	throws DOMException {
	setProperty(CSSConstants.ATTR_UNICODE_BIDI, unicodeBidi, null);
    }

    /**
     *  See the vertical-align property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVerticalAlign() {
	return getPropertyValue(CSSConstants.ATTR_VERTICAL_ALIGN);
    }

    public void setVerticalAlign(String verticalAlign)
	throws DOMException {
	setProperty(CSSConstants.ATTR_VERTICAL_ALIGN, verticalAlign, null);
    }

    /**
     *  See the visibility property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVisibility() {
	return getPropertyValue(CSSConstants.ATTR_VISIBILITY);
    }

    public void setVisibility(String visibility)
	throws DOMException {
	setProperty(CSSConstants.ATTR_VISIBILITY, visibility, null);
    }

    /**
     *  See the voice-family property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVoiceFamily() {
	return getPropertyValue(CSSConstants.ATTR_VOICE_FAMILY);
    }

    public void setVoiceFamily(String voiceFamily)
	throws DOMException {
	setProperty(CSSConstants.ATTR_VOICE_FAMILY, voiceFamily, null);
    }

    /**
     *  See the volume property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getVolume() {
	return getPropertyValue(CSSConstants.ATTR_VOLUMN);
    }

    public void setVolume(String volume)
	throws DOMException {
	setProperty(CSSConstants.ATTR_VOLUMN, volume, null);
    }

    /**
     *  See the white-space property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWhiteSpace() {
	return getPropertyValue(CSSConstants.ATTR_WHITESPACE);
    }

    public void setWhiteSpace(String whiteSpace)
	throws DOMException {
	setProperty(CSSConstants.ATTR_WHITESPACE, whiteSpace, null);
    }

    /**
     *  See the widows property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWidows() {
	return getPropertyValue(CSSConstants.ATTR_WIDOWS);
    }

    public void setWidows(String widows)
	throws DOMException {
	setProperty(CSSConstants.ATTR_WIDOWS, widows, null);
    }

    /**
     *  See the width property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWidth() {
	return getPropertyValue(CSSConstants.ATTR_WIDTH);
    }

    public void setWidth(String width)
	throws DOMException {
	setProperty(CSSConstants.ATTR_WIDTH, width, null);
    }

    /**
     *  See the word-spacing property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getWordSpacing() {
	return getPropertyValue(CSSConstants.ATTR_WORD_SPACING);
    }

    public void setWordSpacing(String wordSpacing)
	throws DOMException {
	setProperty(CSSConstants.ATTR_WORD_SPACING, wordSpacing, null);
    }

    /**
     *  See the z-index property definition in CSS2. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the new value has a syntax error and is 
     *   unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public String getZIndex() {
	return getPropertyValue(CSSConstants.ATTR_ZINDEX);
    }

    public void setZIndex(String zIndex)
	throws DOMException {
	setProperty(CSSConstants.ATTR_ZINDEX, zIndex, null);
    }
}




