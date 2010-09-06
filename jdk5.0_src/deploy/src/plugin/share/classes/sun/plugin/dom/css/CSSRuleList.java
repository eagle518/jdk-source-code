/*
 * @(#)CSSRuleList.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    // Parent style sheet
    private org.w3c.dom.css.CSSStyleSheet parentStyleSheet;

    // Parent rule
    private org.w3c.dom.css.CSSRule parentRule;

    // Owner document
    private org.w3c.dom.Document    document;

    // Owner node
    private org.w3c.dom.Node	    ownerNode;

    /**
     * Construct a CSSRuleList object.
     */
    public CSSRuleList(DOMObject obj,
		       org.w3c.dom.Document document,
		       org.w3c.dom.Node	owner,
		       org.w3c.dom.css.CSSStyleSheet parentStyleSheet, 
		       org.w3c.dom.css.CSSRule parentRule) {
	this.obj = obj;
	this.document = document;
	this.ownerNode = owner;
	this.parentStyleSheet = parentStyleSheet;
	this.parentRule = parentRule;
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
	Object result = obj.getSlot(index);

	if (result != null && result instanceof DOMObject) {
	    Object ret = DOMObjectFactory.createCSSObject((DOMObject)result,
			document, ownerNode, parentStyleSheet, parentRule);
	    if(ret != null && ret instanceof org.w3c.dom.css.CSSRule) 
		return (org.w3c.dom.css.CSSRule)ret;
	}
	return null;
    }
}
