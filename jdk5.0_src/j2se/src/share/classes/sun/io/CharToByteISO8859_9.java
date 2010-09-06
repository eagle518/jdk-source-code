/*
 * @(#)CharToByteISO8859_9.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ISO_8859_9;

/**
 * Tables and data to convert Unicode to ISO8859_9
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteISO8859_9 extends CharToByteSingleByte {

    private final static ISO_8859_9 nioCoder = new ISO_8859_9();

    public String getCharacterEncoding() {
        return "ISO8859_9";
    }

    public CharToByteISO8859_9() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
