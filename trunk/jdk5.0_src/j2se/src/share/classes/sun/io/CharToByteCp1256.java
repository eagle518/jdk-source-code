/*
 * @(#)CharToByteCp1256.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS1256;

/**
 * Tables and data to convert Unicode to Cp1256
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1256 extends CharToByteSingleByte {

    private final static MS1256 nioCoder = new MS1256();

    public String getCharacterEncoding() {
        return "Cp1256";
    }

    public CharToByteCp1256() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
