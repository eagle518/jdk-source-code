/*
 * @(#)RGBColor.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import sun.plugin.dom.*;

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
    private DOMObject obj;
    // Owner document
    private org.w3c.dom.Document document;

    public RGBColor(DOMObject obj, org.w3c.dom.Document document) {
        this.obj = obj;
        this.document = document;
    }

    public org.w3c.dom.css.CSSPrimitiveValue getRed() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_RED),
                                                        document);
    }

    public org.w3c.dom.css.CSSPrimitiveValue getGreen() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_GREEN),
                                                        document);
    }

    public org.w3c.dom.css.CSSPrimitiveValue getBlue() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_BLUE),
                                                        document);
    }

    public String toString() {
        return obj.toString();
    }
}
