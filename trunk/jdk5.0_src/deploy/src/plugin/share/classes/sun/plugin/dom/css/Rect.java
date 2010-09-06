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
 *  The <code>Rect</code> interface is used to represent any rect value. This 
 * interface reflects the values in the underlying style property. Hence, 
 * modifications made to the <code>CSSPrimitiveValue</code> objects modify 
 * the style property. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class Rect implements org.w3c.dom.css.Rect {
    
    private org.w3c.dom.css.CSSPrimitiveValue top;
    private org.w3c.dom.css.CSSPrimitiveValue right;
    private org.w3c.dom.css.CSSPrimitiveValue bottom;
    private org.w3c.dom.css.CSSPrimitiveValue left;

    protected Rect( CSSPrimitiveValue top,
		    CSSPrimitiveValue right,
		    CSSPrimitiveValue bottom,
		    CSSPrimitiveValue left) {
	this.top = top;
	this.left = left;
	this.bottom = bottom;
	this.right = right;
    }

    /**
     *  This attribute is used for the top of the rect. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getTop() {
	return top;
    }

    /**
     *  This attribute is used for the right of the rect. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getRight() {
	return right;
    }

    /**
     *  This attribute is used for the bottom of the rect. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getBottom() {
	return bottom;
    }

    /**
     *  This attribute is used for the left of the rect. 
     */
    public org.w3c.dom.css.CSSPrimitiveValue getLeft() {
	return left;
    }

    public String toString() {
	StringBuffer sb = new StringBuffer("rect(");
	sb.append(top.toString());
	sb.append(' ');
	sb.append(right.toString());
	sb.append(' ');
	sb.append(bottom.toString());
	sb.append(' ');
	sb.append(left.toString());
	sb.append(')');
	return sb.toString();
    }

    /**
     * We expect format of rect(top right bottom left)
     */
    public static Rect newRect(CSSValue parent, String cssText) 
	throws DOMException {
	int start = cssText.indexOf('(');
	int end = cssText.indexOf(')');
	if(start == -1 || end == -1 || start >= end)
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
	    
	String valList = cssText.substring(start + 1, end);
	valList = valList.trim();
	StringTokenizer st = new StringTokenizer(valList);
	String top;
	String right;
	String bottom;
	String left;
	
	try {
	    top = st.nextToken();
	    right = st.nextToken();
	    bottom = st.nextToken();
	    left = st.nextToken();
System.out.println("top: " + top + " right: " + right + " bottom: " + bottom + " left: " + left);
	}catch(NoSuchElementException e) {
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
	}

	if(st.hasMoreTokens())
		throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);

	return new Rect(CSSPrimitiveValue.newCSSPrimitiveValue(parent, top),
			CSSPrimitiveValue.newCSSPrimitiveValue(parent, right),
			CSSPrimitiveValue.newCSSPrimitiveValue(parent, bottom),
			CSSPrimitiveValue.newCSSPrimitiveValue(parent, left));
    }
}
