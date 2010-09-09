/*
 * @(#)ByteToCharCp1149.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
 * A table to convert Cp1149 to Unicode.  This converter differs from
 * Cp871 is one code point, 0x9F, which changes from \u00A4 to \u20AC.
 * @author  Alan Liu
 * @version >= JDK1.2
 */
public class ByteToCharCp1149 extends ByteToCharCp871 {
    public ByteToCharCp1149() {}

    public String getCharacterEncoding() {
        return "Cp1149";
    }

    protected char getUnicode(int byteIndex) {
        // Change single code point with respect to parent.
        // Cast to byte to get sign extension to match byteIndex.
        return (byteIndex == (byte)0x9F) ? '\u20AC' : super.getUnicode(byteIndex);
    }
}

//eof
