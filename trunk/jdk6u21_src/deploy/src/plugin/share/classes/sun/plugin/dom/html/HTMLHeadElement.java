/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

/**
 *  Document head information. See the  HEAD element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLHeadElement extends HTMLElement
				   implements org.w3c.dom.html.HTMLHeadElement {

    public HTMLHeadElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  URI designating a metadata profile. See the  profile attribute 
     * definition in HTML 4.0.
     */
    public String getProfile() {
	return getAttribute(HTMLConstants.ATTR_PROFILE);
    }

    public void setProfile(String profile) {
	setAttribute(HTMLConstants.ATTR_PROFILE, profile);
    }

}

