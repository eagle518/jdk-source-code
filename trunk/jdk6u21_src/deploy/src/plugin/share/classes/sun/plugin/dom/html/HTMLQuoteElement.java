/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Element;
import sun.plugin.dom.DOMObject;

/**
 *  For the <code>Q</code> and <code>BLOCKQUOTE</code> elements. See the  Q 
 * element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLQuoteElement extends HTMLElement 
				    implements org.w3c.dom.html.HTMLQuoteElement {

    public HTMLQuoteElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  A URI designating a source document or message. See the  cite 
     * attribute definition in HTML 4.0.
     */
    public String getCite() {
	return getAttribute(HTMLConstants.ATTR_CITE);
    }

    public void setCite(String cite) {
	setAttribute(HTMLConstants.ATTR_CITE, cite);
    }
}
