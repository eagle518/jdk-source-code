/*
 * @(#)CSSFontFaceRule.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;
import sun.plugin.dom.DOMObjectFactory;

/**
 *  The <code>CSSFontFaceRule</code> interface represents a @font-face rule in 
 * a CSS style sheet. The <code>@font-face</code> rule is used to hold a set 
 * of font descriptions. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */

public final class CSSFontFaceRule extends CSSRule
			implements org.w3c.dom.css.CSSFontFaceRule {

    public CSSFontFaceRule(DOMObject obj, 
		       org.w3c.dom.Document document) {
	super(obj, document);
    }

    /**
     *  The declaration-block of this rule. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getStyle() {
        return DOMObjectFactory.createCSSStyleDeclaration(obj.getMember(CSSConstants.ATTR_STYLE),
                                                          document);
    }

}
