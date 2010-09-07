/*
 * @(#)HTMLCollection.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */


package sun.plugin.dom.html;

import org.w3c.dom.Node;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;


/**
 *  An <code>HTMLCollection</code> is a list of nodes. An individual node may 
 * be accessed by either ordinal index or the node's<code>name</code> or 
 * <code>id</code> attributes.  Note: Collections in the HTML DOM are assumed 
 * to be  live meaning that they are automatically updated when the 
 * underlying document is changed. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLCollection implements org.w3c.dom.html.HTMLCollection,
				org.w3c.dom.NodeList
{
    // Underlying DOMObject
    protected DOMObject obj;

    // Owner document
    protected org.w3c.dom.html.HTMLDocument doc;

    /**
     * Construct a HTML Collection object.
     */
    public HTMLCollection(DOMObject obj, 
			  org.w3c.dom.html.HTMLDocument doc) {
	this.obj = obj;
	this.doc = doc;
    }

    /**
     *  This attribute specifies the length or  size of the list. 
     */
    public int getLength() {
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_LENGTH);	
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
        return DOMObjectFactory.createNode(obj.getSlot(index), doc);
    }

    /**
     *  This method retrieves a <code>Node</code> using a name. It first 
     * searches for a <code>Node</code> with a matching <code>id</code> 
     * attribute. If it doesn't find one, it then searches for a 
     * <code>Node</code> with a matching <code>name</code> attribute, but 
     * only on those elements that are allowed a name attribute. 
     * @param name  The name of the <code>Node</code> to be fetched.
     * @return  The <code>Node</code> with a <code>name</code> or 
     *   <code>id</code> attribute whose value corresponds to the specified 
     *   string. Upon failure (e.g., no node with this name exists), returns 
     *   <code>null</code> .
     */
    public Node namedItem(String name) {
        return DOMObjectFactory.createNode(obj.call(HTMLConstants.FUNC_NAMED_ITEM,
                                                    new Object[] { name }),
                                           doc);
    }
}
