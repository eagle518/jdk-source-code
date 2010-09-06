/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.ArrayList;

/**
 * The <code>CSSValueList</code> interface provides the abstraction of an 
 * ordered collection of CSS values.
 * <p> Some properties allow an empty list into their syntax. In that case, 
 * these properties take the <code>none</code> identifier. So, an empty list 
 * means that the property has the value <code>none</code>. 
 * <p> The items in the <code>CSSValueList</code> are accessible via an 
 * integral index, starting from 0. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSValueList extends CSSValue 
			        implements org.w3c.dom.css.CSSValueList {

    private ArrayList	values = new ArrayList();

    protected CSSValueList(CSSStyleDeclaration css, String propertyName) {
	super(css, propertyName);
    }

    protected CSSValueList(CSSValue parent) {
	super(parent);
    }

    /**
     *  A code defining the type of the value as defined above. 
     */
    public short getCssValueType() {
	return CSS_VALUE_LIST;
    }

    /**
     * The number of <code>CSSValues</code> in the list. The range of valid 
     * values of the indices is <code>0</code> to <code>length-1</code> 
     * inclusive.
     */
    public int getLength() {
	return values.size();
    }

    /**
     * Used to retrieve a <code>CSSValue</code> by ordinal index. The order in 
     * this collection represents the order of the values in the CSS style 
     * property. If index is greater than or equal to the number of values 
     * in the list, this returns <code>null</code>.
     * @param indexIndex into the collection.
     * @return The <code>CSSValue</code> at the <code>index</code> position 
     *   in the <code>CSSValueList</code>, or <code>null</code> if that is 
     *   not a valid index.
     */
    public org.w3c.dom.css.CSSValue item(int index) {
	return (CSSValue)values.get(index);
    }

    public String toString() {
	StringBuffer sb = new StringBuffer();
	int len = getLength();
	for(int index = 0; index < getLength(); index ++) {
	    sb.append(item(index).toString());
	    if(index < len - 1)
		sb.append(' ');
	}

	return sb.toString();
    }

    protected boolean isSameType(CSSValue val) {
	if(val.getCssValueType() != getCssValueType())
	    return false;

	CSSValueList valList = (CSSValueList)val;
	if(valList.getLength() != getLength())
	    return false;

	CSSValue myItemVal;
	for(int index = 0; index < valList.getLength(); index ++) {
	    CSSValue itemVal = (CSSValue)valList.item(index);
	    myItemVal = (CSSValue)item(index);
	    if(!myItemVal.isSameType(itemVal))
		return false;
	}

	return true;
    }

    protected void copy(CSSValue val) {
	super.copy(val);
	CSSValueList valList = (CSSValueList)val;
	values.clear();
	for(int index = 0; index < valList.getLength(); index ++) {
	    addValue((CSSValue)valList.item(index));
	}
    }

    private void addValue(CSSValue value) {
	values.add(value);
    }

    public static CSSValueList newCSSValueList(CSSValue parent,
					       String cssText) 
	throws DOMException {
	CSSValueList valList = new CSSValueList(parent);
	return newCSSValueList(valList, cssText);
    }	

    public static CSSValueList newCSSValueList(CSSStyleDeclaration css, 
					       String propertyName, 
					       String cssText) 
	throws DOMException {
	CSSValueList valList = new CSSValueList(css, propertyName);
	return newCSSValueList(valList, cssText);
    }

    private static CSSValueList newCSSValueList(CSSValueList valList,
						String cssText)
	throws DOMException {
	String cssTexts = cssText.trim();

	String tmpCssText;
	int offset;
	while((offset = getNextCssText(cssTexts)) > 0) {
	System.out.println("Return index: " + offset);
	    if(cssTexts.charAt(offset) != ' ')
		offset ++;
	    tmpCssText = cssTexts.substring(0, offset);
	System.out.println("Return cssText: " + tmpCssText);
	    valList.addValue(CSSValue.newCSSValue(valList, tmpCssText));
	    if(offset >= cssTexts.length())
		break;
	    
	    cssTexts = cssTexts.substring(offset + 1);
	    System.out.println("Rest cssText: " + cssTexts);
	}
	valList.cssText = cssText;
	return valList;
    }

    private static int getNextCssText(String cssTexts) 
	throws DOMException {
	char c;
	int index;
	for(index = 0; index < cssTexts.length(); index ++) {
	    c = cssTexts.charAt(index);
	    switch(c) {
		case ' ':   return index;
		case '(':   index = cssTexts.indexOf(")", index + 1);
			    if(index == -1)
				throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssTexts);
			    break;
		case '\"':  index = cssTexts.indexOf("\"", index + 1);
			    if(index == -1)
				throw new sun.plugin.dom.exception.InvalidStateException("Invalid cssText: " + cssTexts);
			    break;
	    } 
	}
	
	return index - 1;
    }
}
