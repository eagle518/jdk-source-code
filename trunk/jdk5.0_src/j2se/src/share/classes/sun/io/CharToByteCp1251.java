/*
 * @(#)CharToByteCp1251.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.MS1251;

/**
 * Tables and data to convert Unicode to Cp1251
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1251 extends CharToByteSingleByte {

    private final static MS1251 nioCoder = new MS1251();

    public String getCharacterEncoding() {
        return "Cp1251";
    }

    public CharToByteCp1251() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
