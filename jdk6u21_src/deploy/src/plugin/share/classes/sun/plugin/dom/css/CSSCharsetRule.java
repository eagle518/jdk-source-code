/*
 * @(#)CSSCharsetRule.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  The <code>CSSCharsetRule</code> interface represents a @charset rule in a 
 * CSS style sheet. The value of the <code>encoding</code> attribute does 
 * not affect the encoding of text data in the DOM objects; this encoding is 
 * always UTF-16. After a stylesheet is loaded, the value of the 
 * <code>encoding</code> attribute is the value found in the 
 * <code>@charset</code> rule. If there was no <code>@charset</code> in the 
 * original document, then no <code>CSSCharsetRule</code> is created. The 
 * value of the <code>encoding</code> attribute may also be used as a hint 
 * for the encoding used on serialization of the style sheet. 
 * <p> The value of the @charset rule (and therefore of the 
 * <code>CSSCharsetRule</code>) may not correspond to the encoding the 
 * document actually came in; character encoding information e.g. in an HTTP 
 * header, has priority (see CSS document representation) but this is not 
 * reflected in the <code>CSSCharsetRule</code>. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSCharsetRule extends CSSRule 
				  implements org.w3c.dom.css.CSSCharsetRule{

    public CSSCharsetRule(DOMObject obj, 
			  org.w3c.dom.Document document) {
	super(obj, document);
    }

    public short getType() {
	return CHARSET_RULE;
    }

    /**
     *  The encoding information used in this <code>@charset</code> rule. 
     * @exception DOMException
     *   SYNTAX_ERR: Raised if the specified encoding value has a syntax error 
     *   and is unparsable.
     *   <br>NO_MODIFICATION_ALLOWED_ERR: Raised if this encoding rule is 
     *   readonly.
     */
    public String getEncoding() {
	return DOMObjectHelper.getStringMemberNoEx(obj, CSSConstants.ATTR_ENCODING);
    }

    public void setEncoding(String encoding)
	throws DOMException {
	DOMObjectHelper.setStringMemberNoEx(obj, CSSConstants.ATTR_ENCODING, encoding);
    }
}
