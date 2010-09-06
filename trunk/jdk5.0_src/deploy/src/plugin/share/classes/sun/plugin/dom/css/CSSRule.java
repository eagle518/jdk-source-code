/*
 * @(#)CSSRule.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  The <code>CSSRule</code> interface is the abstract base interface for any 
 * type of CSS statement. This includes both rule sets and at-rules. An 
 * implementation is expected to preserve all rules specified in a CSS style 
 * sheet, even if the rule is not recognized by the parser. Unrecognized 
 * rules are represented using the <code>CSSUnknownRule</code> interface. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class CSSRule implements org.w3c.dom.css.CSSRule {
    // Underlying DOMObject
    protected DOMObject obj;

    // Owner document
    protected org.w3c.dom.Document  document;

    // Owner node
    protected org.w3c.dom.Node	    ownerNode;

    // Parent stylesheet
    protected org.w3c.dom.css.CSSStyleSheet parentStyleSheet;

    // Parent rule
    protected org.w3c.dom.css.CSSRule parentRule;

    /**
     * Construct a CSSRule object.
     */
    public CSSRule(DOMObject obj,
		   org.w3c.dom.Document doc,
		   org.w3c.dom.Node	owner,
		   org.w3c.dom.css.CSSStyleSheet parentStyleSheet, 
		   org.w3c.dom.css.CSSRule parentRule) {
	this.obj = obj;
	this.document = doc;
	this.ownerNode = owner;
	this.parentStyleSheet = parentStyleSheet;
	this.parentRule = parentRule;
    }
    
    /**
     *  The type of the rule, as defined above. The expectation is that 
     * binding-specific casting methods can be used to cast down from an 
     * instance of the <code>CSSRule</code> interface to the specific 
     * derived interface implied by the <code>type</code>. 
     */
    public short getType() {
	return UNKNOWN_RULE;
    }

    /**
     *  The parsable textual representation of the rule. This reflects the 
     * current state of the rule and not its initial value. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified CSS string value has a syntax 
     *   error and is unparsable.
     *   <br>INVALID_MODIFICATION_ERR: Raised if the specified CSS string 
     *   value represents a different type of rule than the current one.
     *   <br>HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at 
     *   this point in the style sheet.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if the rule is readonly.
     */
    public String getCssText() {
	return DOMObjectHelper.getStringMemberNoEx(obj, CSSConstants.ATTR_CSS_TEXT);
    }

    public void setCssText(String cssText)
                        throws DOMException {
	DOMObjectHelper.setStringMember(obj, CSSConstants.ATTR_CSS_TEXT, cssText);
    }

    /**
     *  The style sheet that contains this rule. 
     */
    public org.w3c.dom.css.CSSStyleSheet getParentStyleSheet() {
	return parentStyleSheet;
    }

    /**
     *  If this rule is contained inside another rule (e.g. a style rule 
     * inside an @media block), this is the containing rule. If this rule is 
     * not nested inside any other rules, this returns <code>null</code>. 
     */
    public org.w3c.dom.css.CSSRule getParentRule() {
	return parentRule;
    }
}
