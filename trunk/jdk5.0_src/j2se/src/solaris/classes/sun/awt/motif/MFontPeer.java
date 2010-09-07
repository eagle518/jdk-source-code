/*
 * @(#)MFontPeer.java	1.24 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.GraphicsEnvironment;
import sun.awt.PlatformFont;

public class MFontPeer extends PlatformFont {

    /*
     * XLFD name for XFontSet.
     */
    private String xfsname;

    /*
     * converter name for this XFontSet encoding.
     */
    private String converter;

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

    public MFontPeer(String name, int style){
	super(name, style);

	if (fontConfig != null) {
	    xfsname = ((MFontConfiguration) fontConfig).getMotifFontSet(familyName, style);
	}
    }

    protected char getMissingGlyphCharacter() {
        return '\u274F';
    }
}
