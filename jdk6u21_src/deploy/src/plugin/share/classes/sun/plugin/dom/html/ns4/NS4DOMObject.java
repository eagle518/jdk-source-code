/*
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html.ns4;

import netscape.javascript.JSObject;
import sun.plugin.dom.DOMObject;

/**
 * NS4DOMObject is a workaround class for netscape 4
 *
 */
public final class NS4DOMObject extends DOMObject {
    public static final short	TYPE_LINK = 1;
    public static final short	TYPE_INPUT = 2;
    public static final short	TYPE_IMAGE = 3;
    public static final short	TYPE_APPLET = 4;
    public static final short	TYPE_ANCHOR = 5;

    private short type;

    public NS4DOMObject(DOMObject raw, short type) {
	super((JSObject)raw.getJSObject());
	this.type = type;	
    }

    public String toString() {
	switch(type) {
	    case TYPE_LINK:	return "[object HTMLLinkElement]";
	    case TYPE_INPUT:	return "[object HTMLInputElement]";
	    case TYPE_IMAGE:	return "[object HTMLImageElement]";
	    case TYPE_APPLET:	return "[object HTMLAppletElement]";
	    case TYPE_ANCHOR:	return "[object HTMLAnchorElement]";
	}

	return super.toString();
    }
}
