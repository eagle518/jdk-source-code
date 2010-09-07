/*
 * @(#)DocumentFragment.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;

/**
 * <code>DocumentFragment</code> is a "lightweight" or "minimal" 
 * <code>Document</code> object. It is very common to want to be able to 
 * extract a portion of a document's tree or to create a new fragment of a 
 * document. Imagine implementing a user command like cut or rearranging a 
 * document by moving fragments around. It is desirable to have an object 
 * which can hold such fragments and it is quite natural to use a Node for 
 * this purpose. While it is true that a <code>Document</code> object could 
 * fulfill this role, a <code>Document</code> object can potentially be a 
 * heavyweight object, depending on the underlying implementation. What is 
 * really needed for this is a very lightweight object. 
 * <code>DocumentFragment</code> is such an object.
 * <p> Furthermore, various operations -- such as inserting nodes as children 
 * of another <code>Node</code> -- may take <code>DocumentFragment</code> 
 * objects as arguments;  this results in all the child nodes of the 
 * <code>DocumentFragment</code> being moved to the child list of this node.
 * <p> The children of a <code>DocumentFragment</code> node are zero or more 
 * nodes representing the tops of any sub-trees defining the structure of the 
 * document. <code>DocumentFragment</code> nodes do not need to be 
 * well-formed XML documents (although they do need to follow the rules 
 * imposed upon well-formed XML parsed entities, which can have multiple top 
 * nodes). For example, a <code>DocumentFragment</code> might have only one 
 * child and that child node could be a <code>Text</code> node. Such a 
 * structure model represents neither an HTML document nor a well-formed XML 
 * document.
 * <p> When a <code>DocumentFragment</code> is inserted into a 
 * <code>Document</code> (or indeed any other <code>Node</code> that may take 
 * children) the children of the <code>DocumentFragment</code> and not the 
 * <code>DocumentFragment</code> itself are inserted into the 
 * <code>Node</code> . This makes the <code>DocumentFragment</code> very 
 * useful when the user wishes to create nodes that are siblings; the 
 * <code>DocumentFragment</code> acts as the parent of these nodes so that 
 * the user can use the standard methods from the <code>Node</code> 
 * interface, such as <code>insertBefore</code> and <code>appendChild</code> .
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public final class DocumentFragment extends sun.plugin.dom.core.Node 
    implements org.w3c.dom.DocumentFragment
{
    /**
     * Construct a CharacterData object.
     */
    public DocumentFragment(DOMObject obj, org.w3c.dom.Document doc) {
	super(obj, doc);
    }

    /**
     *  The value of this node, depending on its type; see the table above. 
     * When it is defined to be <code>null</code> , setting it has no effect.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @exception DOMException
     *    DOMSTRING_SIZE_ERR: Raised when it would return more characters 
     *   than fit in a <code>DOMString</code> variable on the implementation 
     *   platform.
     */
    public String getNodeValue() throws DOMException {
	throw new sun.plugin.dom.exception.PluginNotSupportedException("DocumentFragment.getNodeValue() is not supported");
    }

    public void setNodeValue(String nodeValue)
	throws DOMException {
	throw new sun.plugin.dom.exception.PluginNotSupportedException("DocumentFragment.setNodeValue() is not supported");
    }
}
