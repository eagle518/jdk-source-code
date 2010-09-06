/*
 * @(#)ByteToCharJIS0212_Solaris.java	1.4	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */


package sun.io;

import sun.nio.cs.ext.JIS_X_0212_Solaris_Decoder;

/**
 * Tables and data to convert JIS0212_Solaris to Unicode
 * Solaris ==> vendor defined characters -- NEC row 13 + IBM extended
 *         selected character set.
 * @author  ConverterGenerator tool
 * @version >= JDK1.3.1
 */

public class ByteToCharJIS0212_Solaris extends ByteToCharDoubleByte {

    public String getCharacterEncoding() {
        return "JIS0212_Solaris";
    }

    protected char convSingleByte(int b) {
        //Fix bug#4179800 - JIS0212 is 7bit,double-byte encoding
        return REPLACE_CHAR;
    }

    public ByteToCharJIS0212_Solaris() {
        super.index1 = JIS_X_0212_Solaris_Decoder.getIndex1();
        super.index2 = JIS_X_0212_Solaris_Decoder.getIndex2();
        start = 0x21;
        end = 0x7E;
    }
}

