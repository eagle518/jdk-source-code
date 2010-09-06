/*
 * @(#)CharToByteCp1025.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.IBM1025;

/**
 * Tables and data to convert Unicode to Cp1025
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1025 extends CharToByteSingleByte {

    private final static IBM1025 nioCoder = new IBM1025();

    public String getCharacterEncoding() {
        return "Cp1025";
    }

    public CharToByteCp1025() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
