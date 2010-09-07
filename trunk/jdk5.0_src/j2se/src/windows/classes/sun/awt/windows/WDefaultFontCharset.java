/*
 * @(#)WDefaultFontCharset.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import sun.io.CharToByteCp1252;

public class WDefaultFontCharset extends CharToByteCp1252
{
    static {
       initIDs();
    }

    // Name for Windows FontSet.
    private String fontName;

    public WDefaultFontCharset(String name){
	fontName = name;
    }

    public synchronized native boolean canConvert(char ch);

    public String toString() {
	return "WDefaultFontCharset";
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();
}
