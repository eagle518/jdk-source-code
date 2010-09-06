/*
 * @(#)MsgNative2ascii_ja.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.native2ascii.resources;

import java.util.ListResourceBundle;

public class MsgNative2ascii_ja extends ListResourceBundle {

    public Object[][] getContents() {
	return contents;
    }

    static final Object[][] contents = {
	{"err.bad.arg", "-encoding \u306b\u306f\u3001\u5f15\u6570\u304c\u5fc5\u8981\u3067\u3059\u3002"},
	{"err.cannot.read",  "{0} \u3092\u8aad\u307f\u8fbc\u3080\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093\u3002"},
	{"err.cannot.write", "{0} \u306b\u66f8\u304d\u8fbc\u3080\u3053\u3068\u304c\u3067\u304d\u307e\u305b\u3093\u3002"},
	{"usage", "\u4f7f\u3044\u65b9: native2ascii" +
	 " [-reverse] [-encoding encoding] [inputfile [outputfile]]"},
    };
}
