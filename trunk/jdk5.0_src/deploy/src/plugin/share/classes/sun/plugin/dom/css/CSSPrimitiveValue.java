/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;

import java.util.HashMap;

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
    private static final String RGB = "rgb";
    private static final String RECT = "rect";
    private static final String COLOR = "#";

    private short   primitiveType = CSS_UNKNOWN;
    private Object  value = null;

    protected CSSPrimitiveValue(CSSStyleDeclaration css, String propertyName) {
	super(css, propertyName);
    }
    
    protected CSSPrimitiveValue(CSSValue parent) {
	super(parent);
    }

    /**
     *  A code defining the type of the value as defined above. 
     */
    public short getCssValueType() {
	return CSS_PRIMITIVE_VALUE;
    }

    /**
     * The type of the value as defined by the constants specified above.
     */
    public short getPrimitiveType() {
	return primitiveType;
    }

    /**
     *  A method to set the float value with a specified unit. If the property 
     * attached with this value can not accept the specified unit or the 
     * float value, the value will be unchanged and a 
     * <code>DOMException</code> will be raised. 
     * @param unitType A unit code as defined above. The unit code can only 
     *   be a float unit type (i.e. <code>CSS_NUMBER</code>, 
     *   <code>CSS_PERCENTAGE</code>, <code>CSS_EMS</code>, 
     *   <code>CSS_EXS</code>, <code>CSS_PX</code>, <code>CSS_CM</code>, 
     *   <code>CSS_MM</code>, <code>CSS_IN</code>, <code>CSS_PT</code>, 
     *   <code>CSS_PC</code>, <code>CSS_DEG</code>, <code>CSS_RAD</code>, 
     *   <code>CSS_GRAD</code>, <code>CSS_MS</code>, <code>CSS_S</code>, 
     *   <code>CSS_HZ</code>, <code>CSS_KHZ</code>, 
     *   <code>CSS_DIMENSION</code>). 
     * @param floatValue The new float value. 
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the attached property doesn't support 
     *   the float value or the unit type.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public void setFloatValue(short unitType, 
                              float floatValue)
                              throws DOMException {
	if(unitType == primitiveType && isFloatType(unitType)) {
	    value = new Float(floatValue);
	    updateProperty();
	} else {
    	    throw new sun.plugin.dom.exception.NoModificationAllowedException("Can not set: " + floatValue);
	}	    
    }

    /**
     *  This method is used to get a float value in a specified unit. If this 
     * CSS value doesn't contain a float value or can't be converted into 
     * the specified unit, a <code>DOMException</code> is raised. 
     * @param unitType A unit code to get the float value. The unit code can 
     *   only be a float unit type (i.e. <code>CSS_NUMBER</code>, 
     *   <code>CSS_PERCENTAGE</code>, <code>CSS_EMS</code>, 
     *   <code>CSS_EXS</code>, <code>CSS_PX</code>, <code>CSS_CM</code>, 
     *   <code>CSS_MM</code>, <code>CSS_IN</code>, <code>CSS_PT</code>, 
     *   <code>CSS_PC</code>, <code>CSS_DEG</code>, <code>CSS_RAD</code>, 
     *   <code>CSS_GRAD</code>, <code>CSS_MS</code>, <code>CSS_S</code>, 
     *   <code>CSS_HZ</code>, <code>CSS_KHZ</code>, 
     *   <code>CSS_DIMENSION</code>). 
     * @return  The float value in the specified unit. 
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the CSS value doesn't contain a float 
     *   value or if the float value can't be converted into the specified 
     *   unit. 
     */
    public float getFloatValue(short unitType)
	throws DOMException {
	if(isFloatType(primitiveType))
	    return ((Float)value).floatValue();	

	throw new sun.plugin.dom.exception.InvalidAccessException("Not a Float value");
    }

    private boolean isFloatType(short type) {
	return (type == CSS_NUMBER || type == CSS_PERCENTAGE || type == CSS_EMS ||
		type == CSS_EXS || type == CSS_PX || type == CSS_CM || type == CSS_MM ||
		type == CSS_IN || type == CSS_PT || type == CSS_PC || type == CSS_DEG ||
		type == CSS_RAD || type == CSS_GRAD || type == CSS_MS || type == CSS_S ||
		type == CSS_HZ || type == CSS_KHZ || type == CSS_DIMENSION);
    }

    /**
     *  A method to set the string value with the specified unit. If the 
     * property attached to this value can't accept the specified unit or 
     * the string value, the value will be unchanged and a 
     * <code>DOMException</code> will be raised. 
     * @param stringType A string code as defined above. The string code can 
     *   only be a string unit type (i.e. <code>CSS_STRING</code>, 
     *   <code>CSS_URI</code>, <code>CSS_IDENT</code>, and 
     *   <code>CSS_ATTR</code>). 
     * @param stringValue The new string value. 
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the CSS value doesn't contain a string 
     *   value or if the string value can't be converted into the specified 
     *   unit.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this property is readonly.
     */
    public void setStringValue(short stringType, 
                               String stringValue)
                               throws DOMException {
	if(primitiveType == stringType && isStringType(stringType)) {
	    value = stringValue;
	    updateProperty();
	} else {
	    throw new sun.plugin.dom.exception.NoModificationAllowedException("Can not set: " + stringValue);
	}
    }

    /**
     *  This method is used to get the string value. If the CSS value doesn't 
     * contain a string value, a <code>DOMException</code> is raised.  Some 
     * properties (like 'font-family' or 'voice-family') convert a 
     * whitespace separated list of idents to a string. 
     * @return  The string value in the current unit. The current 
     *   <code>primitiveType</code> can only be a string unit type (i.e. 
     *   <code>CSS_STRING</code>, <code>CSS_URI</code>, 
     *   <code>CSS_IDENT</code> and <code>CSS_ATTR</code>). 
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the CSS value doesn't contain a string 
     *   value. 
     */
    public String getStringValue()
	    throws DOMException {
	if(isStringType(primitiveType))
	   return (String)value;

	throw new sun.plugin.dom.exception.InvalidAccessException("Not a String value");
    }

    private boolean isStringType(short type) {
	return (CSS_STRING == type || CSS_URI == type || CSS_IDENT == type || CSS_ATTR == type);
    }

    /**
     *  This method is used to get the Counter value. If this CSS value 
     * doesn't contain a counter value, a <code>DOMException</code> is 
     * raised. Modification to the corresponding style property can be 
     * achieved using the <code>Counter</code> interface. 
     * @return The Counter value.
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the CSS value doesn't contain a 
     *   Counter value (e.g. this is not <code>CSS_COUNTER</code>). 
     */
    public org.w3c.dom.css.Counter getCounterValue()
	throws DOMException {
	if(CSS_COUNTER == primitiveType)
	    return (org.w3c.dom.css.Counter)value;

	throw new sun.plugin.dom.exception.InvalidAccessException("Not a Counter value");
    }

    /**
     *  This method is used to get the Rect value. If this CSS value doesn't 
     * contain a rect value, a <code>DOMException</code> is raised. 
     * Modification to the corresponding style property can be achieved 
     * using the <code>Rect</code> interface. 
     * @return The Rect value.
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the CSS value doesn't contain a Rect 
     *   value. (e.g. this is not <code>CSS_RECT</code>). 
     */
    public org.w3c.dom.css.Rect getRectValue()
	throws DOMException {
	if(CSS_RECT == primitiveType) 
	    return (org.w3c.dom.css.Rect)value;
	throw new sun.plugin.dom.exception.InvalidAccessException("Not a Rect value");
    }

    /**
     *  This method is used to get the RGB color. If this CSS value doesn't 
     * contain a RGB color value, a <code>DOMException</code> is raised. 
     * Modification to the corresponding style property can be achieved 
     * using the <code>RGBColor</code> interface. 
     * @return the RGB color value.
     * @exception DOMException
     *    INVALID_ACCESS_ERR: Raised if the attached property can't return a 
     *   RGB color value (e.g. this is not <code>CSS_RGBCOLOR</code>). 
     */
    public org.w3c.dom.css.RGBColor getRGBColorValue()
	throws DOMException {
	if(CSS_RGBCOLOR == primitiveType)
	    return (org.w3c.dom.css.RGBColor)value;
	
	throw new sun.plugin.dom.exception.InvalidAccessException("Not a RGBColor value");	
    }

    public String toString() {
	if(isStringType(primitiveType)) 
	    return (String)value;
	 
	if(isFloatType(primitiveType)) {
	    String unit = (String)getType2UnitMap().get(new Short(primitiveType));
	    if(unit == null)
		return ((Float)value).toString();
	    else 
		return ((Float)value).toString() + unit;
	}

	if(primitiveType == CSS_RECT || primitiveType == CSS_RGBCOLOR)
	    return value.toString();

	if(primitiveType == CSS_COUNTER);
	// what is Counter?

	return null;
    }

    protected boolean isSameType(CSSValue val) {
        return (getCssValueType() == val.getCssValueType() &&
	    ((CSSPrimitiveValue)val).getPrimitiveType() == getPrimitiveType());
    }

    protected void copy(CSSValue val) {
	CSSPrimitiveValue tmp = (CSSPrimitiveValue)val;
	value = tmp.value;
    }

    public static CSSPrimitiveValue newCSSPrimitiveValue(CSSStyleDeclaration css, 
							 String propertyName,
							 String cssText) 
	throws DOMException {
	CSSPrimitiveValue value = new CSSPrimitiveValue(css, propertyName);
	return newCSSPrimitiveValue(value, cssText);
    }

    public static CSSPrimitiveValue newCSSPrimitiveValue(CSSValue parent,
							 String cssText) 
	throws DOMException {
	CSSPrimitiveValue value = new CSSPrimitiveValue(parent);
	return newCSSPrimitiveValue(value, cssText);
    }

    private static CSSPrimitiveValue newCSSPrimitiveValue(CSSPrimitiveValue value,
							  String cssText)
	throws DOMException {

	if(cssText.startsWith(COLOR) || cssText.startsWith(RGB)) {
	     RGBColor color = RGBColor.newRGBColor(value, cssText);
	     value.value = color;
	     value.primitiveType = CSS_RGBCOLOR;
	     return value;
	}

	if(cssText.startsWith(RECT)) {
	    Rect rect = Rect.newRect(value, cssText);
	    value.value = rect;
	    value.primitiveType = CSS_RECT;
	    return value;
	}

	int index = splitCssText(cssText);
	if(index == -1)
	    throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);

	
	// a string
	if(index == 0) {
	    value.value = cssText;
	    value.primitiveType = CSS_STRING;
	    return value;
	}

	if(index == cssText.length()) {
	    value.value = new Float(cssText);
	    value.primitiveType = CSS_NUMBER;
	    return value;
	}

	String txtValue = cssText.substring(0, index);
	String txtUnit = cssText.substring(index);

	value.value = new Float(txtValue);
	Short type = (Short)getUnit2TypeMap().get(txtUnit.toLowerCase());
	if(type == null)
	    value.primitiveType = CSS_UNKNOWN;
	else 
	    value.primitiveType = type.shortValue();

	return value;
    }

    /**
     * try to split css text into number and unit
     * For example: 
     *	    10.0px will be split into 10.0 and px
     * @param cssText
     * @return -1 if can not be split, else the position to split
     */
    private static int splitCssText(String cssText) {
	int	index;
	char	c;
	boolean dotFound = false;
	for(index = 0; index < cssText.length(); index ++) {
	    c = cssText.charAt(index);
	    if(Character.isDigit(c))
		continue;
	    if(c == '.') {
		if(dotFound)
		    return -1;
		dotFound = true;
		continue;
	    }
	    break;
	}

	return index;
    } 

    private static synchronized HashMap getUnit2TypeMap() {
	if(unit2Type == null) {
	    unit2Type = new HashMap();
	    unit2Type.put("%", new Short(CSS_PERCENTAGE));
	    unit2Type.put("ems", new Short(CSS_EMS));
	    unit2Type.put("exs", new Short(CSS_EXS));
	    unit2Type.put("px", new Short(CSS_PX));
	    unit2Type.put("cm", new Short(CSS_CM));
	    unit2Type.put("mm", new Short(CSS_MM));
	    unit2Type.put("in", new Short(CSS_IN));
	    unit2Type.put("pt", new Short(CSS_PT));
	    unit2Type.put("pc", new Short(CSS_PC));
	    unit2Type.put("deg", new Short(CSS_DEG));
	    unit2Type.put("rad", new Short(CSS_RAD));
	    unit2Type.put("grad", new Short(CSS_GRAD));
	    unit2Type.put("ms", new Short(CSS_MS));
	    unit2Type.put("s", new Short(CSS_S));
	    unit2Type.put("hz", new Short(CSS_HZ));
	    unit2Type.put("khz", new Short(CSS_KHZ));
	}

	return unit2Type;
    }

    private static synchronized HashMap getType2UnitMap() {
	if(type2Unit == null) {
	    type2Unit = new HashMap();
	    type2Unit.put(new Short(CSS_PERCENTAGE), "%");
	    type2Unit.put(new Short(CSS_EMS), "ems");
	    type2Unit.put(new Short(CSS_EXS), "exs");
	    type2Unit.put(new Short(CSS_PX), "px");
	    type2Unit.put(new Short(CSS_CM), "cm");
	    type2Unit.put(new Short(CSS_MM), "mm");
	    type2Unit.put(new Short(CSS_IN), "in");
	    type2Unit.put(new Short(CSS_PT), "pt");
	    type2Unit.put(new Short(CSS_PC), "pc");
	    type2Unit.put(new Short(CSS_DEG), "deg");
	    type2Unit.put(new Short(CSS_RAD), "rad");
	    type2Unit.put(new Short(CSS_GRAD), "grad");
	    type2Unit.put(new Short(CSS_MS), "ms");
	    type2Unit.put(new Short(CSS_S), "s");
	    type2Unit.put(new Short(CSS_HZ), "hz");
	    type2Unit.put(new Short(CSS_KHZ), "khz");
	}
	return type2Unit;
    }

    private static HashMap unit2Type = null;
    private static HashMap type2Unit = null;
}
