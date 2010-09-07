/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.dom.html.ns4;

import org.w3c.dom.Node;
import sun.plugin.dom.html.HTMLCollection;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.DOMObjectFactory;

/**
 * This is a workaround class for Netscape 4 browser
 */

public final class HTMLFormCollection extends HTMLCollection {

    /**
     * Construct a HTML Form Collection object.
     */
    public HTMLFormCollection(DOMObject obj, 
			      org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  This method retrieves a node specified by ordinal index. Nodes are 
     * numbered in tree order (depth-first traversal order).
     * @param index  The index of the node to be fetched. The index origin is 
     *   0.
     * @return  The <code>Node</code> at the corresponding position upon 
     *   success. A value of <code>null</code> is returned if the index is 
     *   out of range. 
     */
    public Node item(int index) {
	Object result = obj.getSlot(index);
	
	if(result != null && result instanceof DOMObject) {
    	    return DOMObjectFactory.createHTMLElement(
		    new NS4DOMObject((DOMObject)result, NS4DOMObject.TYPE_INPUT), 
		    doc);
	}

	return null;
    }
}
