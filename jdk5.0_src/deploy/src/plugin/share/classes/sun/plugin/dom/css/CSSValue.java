/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import sun.plugin.dom.exception.NoModificationAllowedException;

/**
 *  The <code>CSSValue</code> interface represents a simple or a complex 
 * value. A <code>CSSValue</code> object only occurs in a context of a CSS 
 * property. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public abstract class CSSValue implements org.w3c.dom.css.CSSValue {
    protected String  cssText = null;
    private String  propertyName = null;
    private CSSStyleDeclaration css = null;
    private CSSValue parentCSSValue = null;

    protected CSSValue(CSSStyleDeclaration css, String propertyName) {
	this.css = css;
	this.propertyName = propertyName;
    }

    protected CSSValue(CSSValue parent) {
	parentCSSValue = parent;
    }

    /**
     *  A string representation of the current value. 
     * @exception DOMException
     *    SYNTAX_ERR: Raised if the specified CSS string value has a syntax 
     *   error (according to the attached property) or is unparsable. 
     *   <br>INVALID_MODIFICATION_ERR: Raised if the specified CSS string 
     *   value represents a different type of values than the values allowed 
     *   by the CSS property.
     *   <br> NO_MODIFICATION_ALLOWED_ERR: Raised if this value is readonly. 
     */
    public String getCssText() {
	return cssText;
    }

    public void setCssText(String cssText)
	throws DOMException {
	CSSValue newVal = null;
	if(parentCSSValue != null)
	    newVal = newCSSValue(parentCSSValue, cssText);
	else
	    newVal = newCSSValue(css, propertyName, cssText);

	if(isSameType(newVal)) {
	    copy(newVal);
	    this.cssText = cssText;
	    CSSValue topVal = this;
	    while(topVal.parentCSSValue != null)
		topVal = topVal.parentCSSValue;
	    topVal.updateProperty();
	} else {
	    throw new NoModificationAllowedException("Can not set cssText: " + cssText);
	}

    }

    public String toString() {
	return cssText;
    }

    protected void updateProperty() {
	css.setProperty(propertyName, toString(), null);
    }
    
    protected abstract boolean isSameType(CSSValue val);

    protected void copy(CSSValue val) {
	cssText = val.cssText;
    }

    public static CSSValue newCSSValue(CSSValue parent, String cssText) 
	throws DOMException {
	String tmpCssText = cssText.trim();
	if(hasMultipleValues(tmpCssText))
	    return CSSValueList.newCSSValueList(parent, tmpCssText);
	else
	    return CSSPrimitiveValue.newCSSPrimitiveValue(parent, tmpCssText);
    }

    public static CSSValue newCSSValue(CSSStyleDeclaration css, String propertyName, String cssText) 
	throws DOMException{
	String tmpCssText = cssText.trim();
System.out.println("New CSSValue for " + propertyName + " = " + cssText);
	if(hasMultipleValues(tmpCssText)) {
	    return CSSValueList.newCSSValueList(css, propertyName, tmpCssText);
	}
	else {
	    return CSSPrimitiveValue.newCSSPrimitiveValue(css, propertyName, tmpCssText);
	}
    }

    private static boolean hasMultipleValues(String cssText) 
	throws DOMException {
	char c;
	for(int index = 0; index < cssText.length(); index ++) {
	    c = cssText.charAt(index);
	    switch(c) {
		case ' ':   return true;
		case '(':   index = cssText.indexOf(")", index + 1);
			    if(index == -1)
				throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
			    break;
		case '\"':  index = cssText.indexOf("\"", index + 1);
			    if(index == -1)
				throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssText);
			    break;
	    }
	}
	return false;   
    }
}