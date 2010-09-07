/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.Element;
import sun.plugin.dom.DOMObject;

/**
 *  Notice of modification to part of a document. See the   INS  and  DEL  
 * element definitions in HTML 4.0. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLModElement extends HTMLElement
				  implements org.w3c.dom.html.HTMLModElement {

    public HTMLModElement(DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  A URI designating a document that describes the reason for the change. 
     * See the  cite attribute definition in HTML 4.0.
     */
    public String getCite() {
	return getAttribute(HTMLConstants.ATTR_CITE);
    }

    public void setCite(String cite) {
	setAttribute(HTMLConstants.ATTR_CITE, cite);
    }

    /**
     *  The date and time of the change. See the  datetime attribute definition
     *  in HTML 4.0.
     */
    public String getDateTime() {
	return getAttribute(HTMLConstants.ATTR_DATETIME);
    }

    public void setDateTime(String dateTime) {
	setAttribute(HTMLConstants.ATTR_DATETIME, dateTime);
    }

}


