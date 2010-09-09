/*
 * @(#)CharToByteCp424.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.IBM424;

/**
 * Tables and data to convert Unicode to Cp424
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp424 extends CharToByteSingleByte {

    private final static IBM424 nioCoder = new IBM424();

    public String getCharacterEncoding() {
        return "Cp424";
    }

    public CharToByteCp424() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
