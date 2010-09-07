/*
 * @(#)HTMLHtmlElement.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;

/**
 *  Root of an HTML document. See the  HTML element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLHtmlElement extends sun.plugin.dom.html.HTMLElement 
			     implements org.w3c.dom.html.HTMLHtmlElement
{
    /**
     * Construct a HTMLHtmlElement object.
     */
    public HTMLHtmlElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Version information about the document's DTD. See the  version 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public String getVersion() {
	return getAttribute(HTMLConstants.ATTR_VERSION);
    }

    public void setVersion(String version) {
	setAttribute(HTMLConstants.ATTR_VERSION, version);
    }
}

