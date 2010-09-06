/*
 * @(#)ByteToCharMS949.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS949;

/**
 * Tables and data to convert MS949 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMS949 extends ByteToCharDoubleByte {
    private static final MS949 nioCoder = new MS949();

    public String getCharacterEncoding() {
        return "MS949";
    }

    public ByteToCharMS949() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0x41;
        end = 0xFE;
    }
}
