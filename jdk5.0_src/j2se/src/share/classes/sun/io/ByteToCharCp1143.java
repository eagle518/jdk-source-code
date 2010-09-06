/*
 * @(#)ByteToCharCp1143.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * A table to convert Cp1143 to Unicode.  This converter differs from
 * Cp278 is one code point, 0x5A, which changes from \u00A4 to \u20AC.
 * @author  Alan Liu
 * @version >= JDK1.2
 */
public class ByteToCharCp1143 extends ByteToCharCp278 {
    public ByteToCharCp1143() {}

    public String getCharacterEncoding() {
        return "Cp1143";
    }

    protected char getUnicode(int byteIndex) {
        // Change single code point with respect to parent.
        // [Careful -- if the code point in question is >= 0x80, make
        //  sure you do the comparison like this: (byteIndex == (byte)0x??)]
        return (byteIndex == 0x5A) ? '\u20AC' : super.getUnicode(byteIndex);
    }
}

//eof
