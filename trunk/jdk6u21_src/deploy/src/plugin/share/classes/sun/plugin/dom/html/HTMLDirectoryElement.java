/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

public final class HTMLDirectoryElement extends HTMLElement 
					implements org.w3c.dom.html.HTMLDirectoryElement {

    public HTMLDirectoryElement(DOMObject obj, 
				org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Reduce spacing between list items. See the  compact attribute 
     * definition in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public boolean getCompact() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_COMPACT);
    }

    public void setCompact(boolean compact) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_COMPACT, compact);
    }
}
