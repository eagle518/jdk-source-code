/*
 * @(#)HTMLCollection.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.html.common;

import org.w3c.dom.Node;

import java.util.ArrayList;
import sun.plugin.dom.exception.PluginNotSupportedException;

/**
 *  An <code>HTMLCollection</code> is a list of nodes. An individual node may 
 * be accessed by either ordinal index or the node's<code>name</code> or 
 * <code>id</code> attributes.  Note: Collections in the HTML DOM are assumed 
 * to be  live meaning that they are automatically updated when the 
 * underlying document is changed. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class HTMLCollection implements org.w3c.dom.NodeList,
					     org.w3c.dom.html.HTMLCollection {

    private ArrayList list = new ArrayList();
    /**
     *  This attribute specifies the length or  size of the list. 
     */
    public int getLength() {
	return list.size();
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
	return (Node)list.get(index);
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
	throw new PluginNotSupportedException("HTMLCollection.namedItem() is not supported.");	
    }

    public void add(Node item) {
	list.add((Object)item);
    }
}
