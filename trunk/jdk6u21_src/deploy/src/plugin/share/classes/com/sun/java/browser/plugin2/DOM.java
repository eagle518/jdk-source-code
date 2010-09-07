/*
 * @(#)DOM.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.plugin2;

import netscape.javascript.JSException;
import netscape.javascript.JSObject;

import java.applet.Applet;
import org.w3c.dom.DOMException;
import sun.plugin.dom.DOMObject;
import sun.plugin.dom.html.HTMLDocument;

/**
 * Provides simple access to the Document Object Model (DOM) of the
 * document containing the given applet.
 */

public class DOM {
    private DOM() {}

    /** Fetches the Document DOM object corresponding to the document
        containing the given applet. */
    public static final org.w3c.dom.Document getDocument(Applet applet)
        throws org.w3c.dom.DOMException {
        try {
            JSObject win = (JSObject) JSObject.getWindow(applet);
            JSObject document = (JSObject) win.getMember("document");
            DOMObject obj = new DOMObject(document);
            // FIXME: do we want to have the HTMLDocument refer to itself as its root document?
            return new HTMLDocument(obj, null);
        } catch (JSException e) {
            throw (DOMException)
                new DOMException(DOMException.NOT_SUPPORTED_ERR,
                                 "Error fetching document for applet")
                .initCause(e);
        }
    }
}
