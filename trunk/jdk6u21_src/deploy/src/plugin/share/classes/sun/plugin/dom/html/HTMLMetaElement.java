/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Element;
import sun.plugin.dom.DOMObject;

/**
 *  This contains generic meta-information about the document. See the  META 
 * element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLMetaElement extends HTMLElement 
				   implements org.w3c.dom.html.HTMLMetaElement {

    public HTMLMetaElement(DOMObject obj, 
			   org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Associated information. See the  content attribute definition in HTML 
     * 4.0.
     */
    public String getContent() {
	return getAttribute(HTMLConstants.ATTR_CONTENT);
    }

    public void setContent(String content) {
	setAttribute(HTMLConstants.ATTR_CONTENT, content);
    }

    /**
     *  HTTP response header name. See the  http-equiv attribute definition in 
     * HTML 4.0.
     */
    public String getHttpEquiv() {
	return getAttribute(HTMLConstants.ATTR_HTTP_EQUIV);
    }

    public void setHttpEquiv(String httpEquiv) {
	setAttribute(HTMLConstants.ATTR_HTTP_EQUIV, httpEquiv);
    }

    /**
     *  Meta information name. See the  name attribute definition in HTML 4.0.
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  Select form of content. See the  scheme attribute definition in HTML 
     * 4.0.
     */
    public String getScheme() {
	return getAttribute(HTMLConstants.ATTR_SCHEME);
    }

    public void setScheme(String scheme) {
	setAttribute(HTMLConstants.ATTR_SCHEME, scheme);
    }

}

