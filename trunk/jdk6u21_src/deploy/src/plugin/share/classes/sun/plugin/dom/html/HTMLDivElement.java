/*
 *
 * Copyright 2006 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

public final class HTMLDivElement extends HTMLElement
				  implements org.w3c.dom.html.HTMLDivElement {

    public HTMLDivElement(DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Horizontal text alignment. See the  align attribute definition in HTML 
     * 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }
}