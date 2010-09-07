/*
 * @(#)CSSMediaRule.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

import org.w3c.dom.DOMException;
import org.w3c.dom.stylesheets.MediaList;

/**
 *  The <code>CSSMediaRule</code> interface represents a @media rule in a CSS 
 * style sheet. A <code>@media</code> rule can be used to delimit style 
 * rules for specific media types. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSMediaRule extends CSSRule 
	implements org.w3c.dom.css.CSSMediaRule {

    public CSSMediaRule(DOMObject obj, 
		       org.w3c.dom.Document document) {
	super(obj, document);
    }
   /**
     *  A list of media types for this rule. 
     */
    public org.w3c.dom.stylesheets.MediaList getMedia() {
        try {
            return DOMObjectFactory.createMediaList(obj.getMember(CSSConstants.ATTR_MEDIA),
                                                    document);
        } catch (DOMException e) {
        }
        return null;
    }

    /**
     *  A list of all CSS rules contained within the media block. 
     */
    public org.w3c.dom.css.CSSRuleList getCssRules() {
        return DOMObjectFactory.createCSSRuleList(obj.getMember(CSSConstants.ATTR_CSS_RULES),
                                                  document);
    }

    /**
     *  Used to insert a new rule into the media block. 
     * @param rule The parsable text representing the rule. For rule sets 
     *   this contains both the selector and the style declaration. For 
     *   at-rules, this specifies both the at-identifier and the rule 
     *   content. 
     * @param index The index within the media block's rule collection of the 
     *   rule before which to insert the specified rule. If the specified 
     *   index is equal to the length of the media blocks's rule collection, 
     *   the rule will be added to the end of the media block. 
     * @return  The index within the media block's rule collection of the 
     *   newly inserted rule. 
     * @exception DOMException
     *   HIERARCHY_REQUEST_ERR: Raised if the rule cannot be inserted at the 
     *   specified index, e.g., if an <code>@import</code> rule is inserted 
     *   after a standard rule set or other at-rule.
     *   <br>INDEX_SIZE_ERR: Raised if the specified index is not a valid 
     *   insertion point.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this media rule is 
     *   readonly.
     *   <br>SYNTAX_ERR: Raised if the specified rule has a syntax error and 
     *   is unparsable.
     */
    public int insertRule(String rule, 
                          int index)
                          throws DOMException {

	String ret = null;
	try {
	    ret = DOMObjectHelper.callStringMethod(obj, CSSConstants.FUNC_ADD_RULE, new Object[]{new Integer(index)});
	}catch(DOMException e) {
	    ret = DOMObjectHelper.callStringMethod(obj, CSSConstants.FUNC_INSERT_RULE, new Object[]{new Integer(index)});
	}

	if(ret != null) {
	    return Integer.parseInt(ret);
	}

	return 0;
    }

    /**
     *  Used to delete a rule from the media block. 
     * @param index The index within the media block's rule collection of the 
     *   rule to remove. 
     * @exception DOMException
     *   INDEX_SIZE_ERR: Raised if the specified index does not correspond to 
     *   a rule in the media rule list.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this media rule is 
     *   readonly.
     */
    public void deleteRule(int index)
                           throws DOMException {
	try {
	    obj.call(CSSConstants.FUNC_REMOVE_RULE, new Object[]{new Integer(index)});
	}catch(DOMException e) {
	    obj.call(CSSConstants.FUNC_DELETE_RULE, new Object[]{new Integer(index)});
	}
    }
}
