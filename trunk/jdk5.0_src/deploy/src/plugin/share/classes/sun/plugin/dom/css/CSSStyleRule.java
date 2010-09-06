/*
 * @(#)CSSStyleRule.java	1.4 03/12/19
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

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;


/**
 *  The <code>CSSStyleRule</code> interface represents a single rule set in a 
 * CSS style sheet. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSStyleRule extends sun.plugin.dom.css.CSSRule 
			  implements org.w3c.dom.css.CSSStyleRule
{
    /**
     * Construct a CSSStyleRule object.
     */
    public CSSStyleRule(DOMObject obj,
			org.w3c.dom.Document doc,
			org.w3c.dom.Node owner,
			org.w3c.dom.css.CSSStyleSheet parentStyleSheet, 
			org.w3c.dom.css.CSSRule parentRule) {
	super(obj, doc, owner, parentStyleSheet, parentRule);
    }   

    /**
     *  The textual representation of the selector for the rule set. The 
     * implementation may have stripped out insignificant whitespace while 
     * parsing the selector. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified CSS string value has a syntax 
     *   error and is unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this rule is readonly.
     */
    public String getSelectorText()
    {
	return DOMObjectHelper.getStringMemberNoEx(obj, CSSConstants.ATTR_SELECTOR_TEXT);
    }

    public void setSelectorText(String selectorText)
                        throws DOMException
    {
	DOMObjectHelper.setStringMember(obj, CSSConstants.ATTR_SELECTOR_TEXT, selectorText);
    }

    /**
     *  The type of the rule, as defined above. The expectation is that 
     * binding-specific casting methods can be used to cast down from an 
     * instance of the <code>CSSRule</code> interface to the specific 
     * derived interface implied by the <code>type</code>. 
     */
    public short getType() {
	return STYLE_RULE;
    }
        /**
     *  The declaration-block of this rule set. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getStyle() {
	try {
	    Object result = obj.getMember(CSSConstants.ATTR_STYLE);	
	    if (result != null && result instanceof DOMObject) {
		Object ret = DOMObjectFactory.createCSSObject((DOMObject)result,
							      document,
							      ownerNode,
							      parentStyleSheet,
							      parentRule);
		if(ret != null && ret instanceof org.w3c.dom.css.CSSStyleDeclaration)
		    return (org.w3c.dom.css.CSSStyleDeclaration)ret;
	    }
	} catch (DOMException e){
	}

	return null;
    }
}
