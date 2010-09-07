/*
 * @(#)Text.java	1.12 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;
import sun.plugin.dom.exception.*;

/**
 *  The <code>Text</code> interface inherits from <code>CharacterData</code> 
 * and represents the textual content (termed  character  data in XML) of an 
 * <code>Element</code> or <code>Attr</code> .  If there is no markup inside 
 * an element's content, the text is contained in a single object 
 * implementing the <code>Text</code> interface that is the only child of the 
 * element. If there is markup, it is parsed into the  information items 
 * (elements,  comments, etc.) and <code>Text</code>  nodes that form the 
 * list of children of the element.
 * <p> When a document is first made available via the DOM, there is  only one 
 * <code>Text</code> node for each block of text. Users may create  adjacent 
 * <code>Text</code> nodes that represent the  contents of a given element 
 * without any intervening markup, but should be aware that there is no way 
 * to represent the separations between these nodes in XML or HTML, so they 
 * will not (in general) persist between DOM editing sessions. The 
 * <code>normalize()</code> method on <code>Node</code> merges any such 
 * adjacent <code>Text</code> objects into a single node for each block of 
 * text.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class Text extends sun.plugin.dom.core.CharacterData 
		  implements org.w3c.dom.Text {
    /**
     * Construct a Text object.
     */
    public Text(DOMObject obj, org.w3c.dom.Document doc) {
	super(obj, doc);
    }

    /**
     *  Breaks this node into two  nodes at the specified <code>offset</code> 
     * , keeping both in the tree as siblings. This node then only contains 
     * all the content up to the <code>offset</code> point. A new node of the 
     * same type, which is inserted as the next sibling of this node, 
     * contains all the content at and after the <code>offset</code> point. 
     * When the <code>offset</code> is equal to the length of this node, the 
     * new node has no data.
     * @param offset  The  16-bit unit offset at which to split, starting from 
     *   <code>0</code> .
     * @return  The new node, of the same type as this node.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified offset is negative or 
     *   greater than the number of 16-bit units in <code>data</code> .
     *   <br> NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     */
    public org.w3c.dom.Text splitText(int offset)
			    throws DOMException {
	Object[] args = {new Integer(offset)};

	Object result = obj.call(CoreConstants.FUNC_SPLIT_TEXT, args);
	if (result != null && result instanceof DOMObject) {
	    return new sun.plugin.dom.core.Text((DOMObject) result, getOwnerDocument());
	} else {
	    return null;    
	}
    }

    // Start : DOM L3 Stub methods
    public boolean isElementContentWhitespace() {
        throw new PluginNotSupportedException("Node.isElementContentWhitespace() is not supported.");
    }

    public String getWholeText() {
        throw new PluginNotSupportedException("Node.getWholeText() is not supported.");
    }

    public org.w3c.dom.Text replaceWholeText(String content)
                                 throws DOMException {
        throw new PluginNotSupportedException("Node.replaceWholeText() is not supported.");
    }
    // End: DOM L3 Stub methods
 
 
}

