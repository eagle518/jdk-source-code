/*
 * @(#)XFontPeer.java	1.7 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import sun.awt.PlatformFont;
import java.awt.GraphicsEnvironment;

/* FIX ME */
import sun.awt.motif.MFontConfiguration;

public class XFontPeer extends PlatformFont {

    /*
     * XLFD name for XFontSet.
     */
    private String xfsname;

    static {
        if (!GraphicsEnvironment.isHeadless()) {
            initIDs();
        }
    }

    /**
     * Initialize JNI field and method IDs for fields that may be
       accessed from C.
     */
    private static native void initIDs();

    public XFontPeer(String name, int style){
	super(name, style);

	if (fontConfig != null){
	    xfsname = ((MFontConfiguration) fontConfig).getMotifFontSet(familyName, style);
	}
    }

    protected char getMissingGlyphCharacter() {
        return '\u274F';
    }
}
