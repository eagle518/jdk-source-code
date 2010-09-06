/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
/**
 *  The <code>RGBColor</code> interface is used to represent any RGB color 
 * value. This interface reflects the values in the underlying style 
 * property. Hence, modifications made to the <code>CSSPrimitiveValue</code> 
 * objects modify the style property. 
 * <p> A specified RGB color is not clipped (even if the number is outside the 
 * range 0-255 or 0%-100%). A computed RGB color is clipped depending on the 
 * device. 
 * <p> Even if a style sheet can only contain an integer for a color value, 
 * the internal storage of this integer is a float, and this can be used as 
 * a float in the specified or the computed style. 
 * <p> A color percentage value can always be converted to a number and vice 
 * versa. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class RGBColor implements org.w3c.dom.css.RGBColor {

    private org.w3c.dom.css.CSSPrimitiveValue red;
    private org.w3c.dom.css.CSSPrimitiveValue green;
    private org.w3c.dom.css.CSSPrimitiveValue blue;

    protected RGBColor(org.w3c.dom.css.CSSPrimitiveValue red,
		       org.w3c.dom.css.CSSPrimitiveValue green,
		       org.w3c.dom.css.CSSPrimitiveValue blue) {
	this.red = red;
	this.green = green;
	this.blue = blue;
    }

    /**
     *  This attribute is used for the red value of the RGB color. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getRed() {
	return red;
    }

    /**
     *  This attribute is used for the green value of the RGB color. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getGreen() {
	return green;
    }

    /**
     *  This attribute is used for the blue value of the RGB color. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getBlue() {
	return blue;
    }

    public String toString() {
	StringBuffer sb = new StringBuffer("rgb(");
	int val = (int)red.getFloatValue(CSSPrimitiveValue.CSS_NUMBER);
	sb.append(val);
	sb.append(',');
	val = (int)green.getFloatValue(CSSPrimitiveValue.CSS_NUMBER);
	sb.append(val);
	sb.append(',');
	val = (int)blue.getFloatValue(CSSPrimitiveValue.CSS_NUMBER);
	sb.append(val);
	sb.append(')');
	return sb.toString();
    }

    /**
     * Expect RGB in following two formats:
     *  #rrggbb
     *	rgb(rrr,ggg,bbb)
     */
    public static RGBColor newRGBColor(CSSValue parent, String cssText)
	throws DOMException {

	String red;
	String green;
	String blue;
		
	// #rrggbb
	if(cssText.charAt(0) == '#') {
	    try {
		red = cssText.substring(1, 3);
		green = cssText.substring(3, 5);
		blue = cssText.substring(5, 7);
		red = Integer.valueOf(red, 16).toString();
		green = Integer.valueOf(green, 16).toString();
		blue = Integer.valueOf(blue, 16).toString();
	    }catch(IndexOutOfBoundsException e) {
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
	    }

	} else {    // rgb(rrr,ggg,bbb)
	    int start = cssText.indexOf('(');
	    int end = cssText.indexOf(')');
	    if(start == -1 || end == -1 || start >= end)
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
	    StringTokenizer st = new StringTokenizer(cssText.substring(start + 1, end - 1), ",");
	    try {
		red = st.nextToken();
		green = st.nextToken();
		blue = st.nextToken();
	    } catch(NoSuchElementException e) {
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
	    }

	}

	return new RGBColor(CSSPrimitiveValue.newCSSPrimitiveValue(parent, red),
			    CSSPrimitiveValue.newCSSPrimitiveValue(parent, green),
			    CSSPrimitiveValue.newCSSPrimitiveValue(parent, blue));
    }
}
