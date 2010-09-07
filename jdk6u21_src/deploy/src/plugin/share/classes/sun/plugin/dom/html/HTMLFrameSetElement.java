/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import sun.plugin.dom.DOMObject;

/**
 *  Create a grid of frames. See the  FRAMESET element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLFrameSetElement extends HTMLElement
				       implements org.w3c.dom.html.HTMLFrameSetElement {
    public HTMLFrameSetElement(DOMObject obj, 
				org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  The number of columns of frames in the frameset. See the  cols 
     * attribute definition in HTML 4.0.
     */
    public String getCols() {
	return getAttribute(HTMLConstants.ATTR_COLS);
    }

    public void setCols(String cols) {
	setAttribute(HTMLConstants.ATTR_COLS, cols);
    }

    /**
     *  The number of rows of frames in the frameset. See the  rows attribute 
     * definition in HTML 4.0.
     */
    public String getRows() {
	return getAttribute(HTMLConstants.ATTR_ROWS);
    }

    public void setRows(String rows) {
	setAttribute(HTMLConstants.ATTR_ROWS, rows);
    }

}

