/*
 * @(#)CSSRuleList.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 

package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import org.w3c.dom.css.*;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  The <code>CSSRuleList</code> interface provides the abstraction of an 
 * ordered collection of CSS rules. 
 * <p> The items in the <code>CSSRuleList</code> are accessible via an 
 * integral index, starting from 0. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSRuleList implements org.w3c.dom.css.CSSRuleList {
    // Underlying DOMObject
    private DOMObject obj;

    // Owner document
    private org.w3c.dom.Document document;

    /**
     * Construct a CSSRuleList object.
     */
    public CSSRuleList(DOMObject obj,
		       org.w3c.dom.Document document) {
	this.obj = obj;
	this.document = document;
    }

    /**
     *  The number of <code>CSSRules</code> in the list. The range of valid 
     * child rule indices is <code>0</code> to <code>length-1</code> 
     * inclusive. 
     */
    public int getLength() {
	return DOMObjectHelper.getIntMemberNoEx(obj, CSSConstants.ATTR_LENGTH);	
    }

    /**
     *  Used to retrieve a CSS rule by ordinal index. The order in this 
     * collection represents the order of the rules in the CSS style sheet. 
     * If index is greater than or equal to the number of rules in the list, 
     * this returns <code>null</code>. 
     * @param indexIndex into the collection
     * @return The style rule at the <code>index</code> position in the 
     *   <code>CSSRuleList</code>, or <code>null</code> if that is not a 
     *   valid index. 
     */
    public org.w3c.dom.css.CSSRule item(int index) {
        return DOMObjectFactory.createCSSRule(obj.getSlot(index), document);
    }
}
