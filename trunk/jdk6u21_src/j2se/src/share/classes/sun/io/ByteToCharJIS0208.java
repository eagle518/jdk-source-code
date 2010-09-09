/*
 * @(#)ByteToCharJIS0208.java	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

/**
 * Tables and data to convert JIS0208 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

import sun.nio.cs.ext.JIS_X_0208_Decoder;

public class ByteToCharJIS0208 extends ByteToCharDoubleByte {

    public String getCharacterEncoding() {
        return "JIS0208";
    }

    public ByteToCharJIS0208() {
	super.index1 = JIS_X_0208_Decoder.getIndex1();
	super.index2 = JIS_X_0208_Decoder.getIndex2();
        start = 0x21;
        end = 0x7E;
    }

    protected char convSingleByte(int b) {
	//Fix bug#4179800 - JIS0208 is 7bit,double-byte encoding
	return REPLACE_CHAR;
    }
}
