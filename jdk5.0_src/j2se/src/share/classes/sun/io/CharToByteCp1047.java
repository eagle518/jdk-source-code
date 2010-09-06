/*
 * @(#)CharToByteCp1047.java	1.4	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.IBM1047;

/**
 * Tables and data to convert Unicode to Cp1047
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1047 extends CharToByteSingleByte {

    private final static IBM1047 nioCoder = new IBM1047();

    public String getCharacterEncoding() {
        return "Cp1047";
    }

    public CharToByteCp1047() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
