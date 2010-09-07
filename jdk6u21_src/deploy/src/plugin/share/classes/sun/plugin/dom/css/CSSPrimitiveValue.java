/*
 * @(#)CSSPrimitiveValue.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;

import java.util.HashMap;
import sun.plugin.dom.*;

/**
 *  The <code>CSSPrimitiveValue</code> interface represents a single CSS value
 * . This interface may be used to determine the value of a specific style 
 * property currently set in a block or to set a specific style property 
 * explicitly within the block. An instance of this interface might be 
 * obtained from the <code>getPropertyCSSValue</code> method of the 
 * <code>CSSStyleDeclaration</code> interface. A 
 * <code>CSSPrimitiveValue</code> object only occurs in a context of a CSS 
 * property. 
 * <p> Conversions are allowed between absolute values (from millimeters to 
 * centimeters, from degrees to radians, and so on) but not between relative 
 * values. (For example, a pixel value cannot be converted to a centimeter 
 * value.) Percentage values can't be converted since they are relative to 
 * the parent value (or another property value). There is one exception for 
 * color percentage values: since a color percentage value is relative to 
 * the range 0-255, a color percentage value can be converted to a number; 
 * (see also the <code>RGBColor</code> interface). 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSPrimitiveValue extends CSSValue 
				     implements org.w3c.dom.css.CSSPrimitiveValue {

    public CSSPrimitiveValue(DOMObject obj, org.w3c.dom.Document document) {
        super(obj, document);
    }

    public short getPrimitiveType() {
        return ((Number) obj.getMember(CSSConstants.ATTR_PRIMITIVE_TYPE)).shortValue();
    }

    public void setFloatValue(short unitType, 
                              float floatValue) throws DOMException {
        obj.call(CSSConstants.FUNC_SET_FLOAT_VALUE,
                 new Object[] { new Integer(unitType), new Float(floatValue) });
    }
    
    public float getFloatValue(short unitType) throws DOMException {
        return ((Number) obj.call(CSSConstants.FUNC_GET_FLOAT_VALUE,
                                  new Object[] { new Integer(unitType) })).floatValue();
    }

    public void setStringValue(short stringType, 
                               String stringValue) throws DOMException {
        obj.call(CSSConstants.FUNC_SET_FLOAT_VALUE,
                 new Object[] { new Integer(stringType), stringValue });
    }

    public String getStringValue() throws DOMException {
        return (String) obj.call(CSSConstants.FUNC_GET_STRING_VALUE, null);
    }

    public org.w3c.dom.css.Counter getCounterValue() throws DOMException {
        return DOMObjectFactory.createCSSCounter(obj.call(CSSConstants.FUNC_GET_COUNTER_VALUE, null));
    }

    public org.w3c.dom.css.Rect getRectValue() throws DOMException {
        return DOMObjectFactory.createCSSRect(obj.call(CSSConstants.FUNC_GET_RECT_VALUE, null),
                                              document);
    }

    public org.w3c.dom.css.RGBColor getRGBColorValue() throws DOMException {
        return DOMObjectFactory.createCSSRGBColor(obj.call(CSSConstants.FUNC_GET_RGB_COLOR_VALUE, null),
                                                  document);
    }
}
