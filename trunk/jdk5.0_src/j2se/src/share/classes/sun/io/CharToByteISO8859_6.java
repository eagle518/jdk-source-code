/*
 * @(#)CharToByteISO8859_6.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.ISO_8859_6;

/**
 * Tables and data to convert Unicode to ISO8859_6
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteISO8859_6 extends CharToByteSingleByte {

    private final static ISO_8859_6 nioCoder = new ISO_8859_6();

    public String getCharacterEncoding() {
        return "ISO8859_6";
    }

    public CharToByteISO8859_6() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
