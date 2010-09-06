/*
 * @(#)CharToByteISO8859_15.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 1998 - All Rights Reserved
 *
 */

package sun.io;

import sun.nio.cs.ISO_8859_15;

/**
 * Tables and data to convert Unicode to ISO8859_15
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.7
 */

public class CharToByteISO8859_15 extends CharToByteSingleByte {

    private final static ISO_8859_15 nioCoder = new ISO_8859_15();

    public String getCharacterEncoding() {
        return "ISO8859_15";
    }

    public CharToByteISO8859_15() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
