/*
 * @(#)NamedNodeMap.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;

/** A class that implements the org.w3c.dom.NamedNodeMap interface. */

public class NamedNodeMap implements org.w3c.dom.NamedNodeMap {
    private DOMObject obj;
    // Parent document
    private org.w3c.dom.html.HTMLDocument document;


    public NamedNodeMap(DOMObject obj, org.w3c.dom.html.HTMLDocument document) {
        this.obj = obj;
        this.document = document;
    }

    public org.w3c.dom.Node getNamedItem(String name) {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_GET_NAMED_ITEM,
                                                    new Object[] { name }),
                                           document);
    }

    public org.w3c.dom.Node setNamedItem(org.w3c.dom.Node arg) throws DOMException {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_SET_NAMED_ITEM,
                                                    new Object[] { arg }),
                                           document);
    }

    public org.w3c.dom.Node removeNamedItem(String name) throws DOMException {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_REMOVE_NAMED_ITEM,
                                                    new Object[] { name }),
                                           document);
    }

    public org.w3c.dom.Node item(int index) {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_ITEM,
                                                    new Object[] { new Integer(index) }),
                                           document);
    }

    public int getLength() {
        return ((Number) obj.getMember(CoreConstants.ATTR_LENGTH)).intValue();
    }

    public org.w3c.dom.Node getNamedItemNS(String namespaceURI, 
                                           String localName) throws DOMException {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_GET_NAMED_ITEM_NS,
                                                    new Object[] { namespaceURI, localName }),
                                           document);
    }

    public org.w3c.dom.Node setNamedItemNS(org.w3c.dom.Node arg) throws DOMException {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_SET_NAMED_ITEM_NS,
                                                    new Object[] { arg }),
                                           document);
    }

    public org.w3c.dom.Node removeNamedItemNS(String namespaceURI, 
                                              String localName) throws DOMException {
        return DOMObjectFactory.createNode(obj.call(CoreConstants.FUNC_REMOVE_NAMED_ITEM_NS,
                                                    new Object[] { namespaceURI, localName }),
                                           document);
    }
}
