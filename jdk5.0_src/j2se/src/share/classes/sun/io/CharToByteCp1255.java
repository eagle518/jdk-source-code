/*
 * @(#)CharToByteCp1255.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS1255;

/**
 * Tables and data to convert Unicode to Cp1255
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1255 extends CharToByteSingleByte {

    private final static MS1255 nioCoder = new MS1255();

    public String getCharacterEncoding() {
        return "Cp1255";
    }

    public CharToByteCp1255() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
