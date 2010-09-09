/*
 * @(#)CharToByteCp1046.java	1.15	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.IBM1046;

/**
 * Tables and data to convert Unicode to Cp1046
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp1046 extends CharToByteSingleByte {

    private final static IBM1046 nioCoder = new IBM1046();

    public String getCharacterEncoding() {
        return "Cp1046";
    }

    public CharToByteCp1046() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
