/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectHelper;

/**
 *  Style information. See the  STYLE element definition in HTML 4.0, the  
 * module and the <code>LinkStyle</code> interface in the  module. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLStyleElement extends HTMLElement 
				    implements org.w3c.dom.html.HTMLStyleElement {

    public HTMLStyleElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  Enables/disables the style sheet. 
     */
    public boolean getDisabled() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DISABLED);
    }

    public void setDisabled(boolean disabled) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DISABLED, disabled);
    }

    /**
     *  Designed for use with one or more target media. See the  media 
     * attribute definition in HTML 4.0.
     */
    public String getMedia() {
	return getAttribute(HTMLConstants.ATTR_MEDIA);
    }

    public void setMedia(String media) {
	setAttribute(HTMLConstants.ATTR_MEDIA, media);
    }

    /**
     *  The content type pf the style sheet language. See the  type attribute 
     * definition in HTML 4.0.
     */
    public String getType() {
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

}
