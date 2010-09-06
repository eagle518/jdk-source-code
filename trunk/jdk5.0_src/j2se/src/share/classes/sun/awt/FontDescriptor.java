/*
 * @(#)FontDescriptor.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import sun.io.CharToByteConverter;
import sun.io.CharToByteUnicode;

public class FontDescriptor implements Cloneable {

    static {
        NativeLibLoader.loadLibraries();
        initIDs();
    }

    String nativeName;
    public CharToByteConverter fontCharset;
    private int[] exclusionRanges;

    public FontDescriptor(String nativeName, CharToByteConverter fontCharset,
			  int[] exclusionRanges){

	this.nativeName = nativeName;
	this.fontCharset = fontCharset;
	this.exclusionRanges = exclusionRanges;
	this.useUnicode = false;
    }
    
    public String getNativeName() {
        return nativeName;
    }
    
    public CharToByteConverter getFontCharset() {
        return fontCharset;
    }

    public int[] getExclusionRanges() {
        return exclusionRanges;
    }

    /**
     * Return true if the character is exclusion character.
     */
    public boolean isExcluded(char ch){
	for (int i = 0; i < exclusionRanges.length; ){

	    int lo = (exclusionRanges[i++]);
	    int up = (exclusionRanges[i++]);

	    if (ch >= lo && ch <= up){
		return true;
	    }
	}
	return false;
    }

    public String toString() {
	return super.toString() + " [" + nativeName + "|" + fontCharset + "]";
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();


    public CharToByteConverter unicodeCharset;
    boolean useUnicode; // set to true from native code on Unicode-based systems
   
    public boolean useUnicode() {
        if (useUnicode && unicodeCharset == null) {
            this.unicodeCharset = new CharToByteUnicode(false);
 	}
        return useUnicode;
    }    
}

