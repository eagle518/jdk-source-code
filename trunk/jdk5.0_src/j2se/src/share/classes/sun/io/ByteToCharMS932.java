/*
 * @(#)ByteToCharMS932.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * Tables and data to convert MS932 to Unicode
 *
 * @author Limin Shi
 * @author Mark Son-Bell
 * @version >= JDK1.1.6
 */

public class ByteToCharMS932 extends ByteToCharMS932DB {
    ByteToCharJIS0201 bcJIS0201 = new ByteToCharJIS0201();

    public String getCharacterEncoding() {
        return "MS932";
    }

    protected char convSingleByte(int b) {
        // If the high bits are all off, it's ASCII == Unicode
        if ((b & 0xFF80) == 0) {
	    return (char)b;
        }
	return bcJIS0201.getUnicode(b);
    }

    String prt(int i) {
	return Integer.toString(i,16);
    }
}
