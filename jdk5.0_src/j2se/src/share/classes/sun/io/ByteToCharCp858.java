/*
 * @(#)ByteToCharCp858.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * A table to convert Cp858 to Unicode.  This converter differs from
 * Cp850 is one code point, 0xD5, which changes from \u0131 to \u20AC.
 * @author  Alan Liu
 * @version >= JDK1.2
 */
public class ByteToCharCp858 extends ByteToCharCp850 {
    public ByteToCharCp858() {}

    public String getCharacterEncoding() {
        return "Cp858";
    }

    protected char getUnicode(int byteIndex) {
        // Change single code point with respect to parent.
        // Cast to byte to get sign extension to match byteIndex.
        return (byteIndex == (byte)0xD5) ? '\u20AC' : super.getUnicode(byteIndex);
    }
}

//eof
