/*
 * @(#)ByteToCharJIS0212.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.JIS_X_0212_Decoder; 

/**
 * Tables and data to convert JIS0212 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */


public class ByteToCharJIS0212 extends ByteToCharDoubleByte {

    public String getCharacterEncoding() {
        return "JIS0212";
    }

    protected char convSingleByte(int b) {
        //Fix bug#4179800 - JIS0212 is 7bit,double-byte encoding
	return REPLACE_CHAR;
    }

    public ByteToCharJIS0212() {
        super.index1 = JIS_X_0212_Decoder.getIndex1();
        super.index2 = JIS_X_0212_Decoder.getIndex2();
        start = 0x21;
        end = 0x7E;
     }
}
