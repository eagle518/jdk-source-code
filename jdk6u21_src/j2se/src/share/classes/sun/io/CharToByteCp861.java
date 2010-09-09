/*
 * @(#)CharToByteCp861.java	1.16	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.IBM861;

/**
 * Tables and data to convert Unicode to Cp861
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteCp861 extends CharToByteSingleByte {

    private final static IBM861 nioCoder = new IBM861();

    public String getCharacterEncoding() {
        return "Cp861";
    }

    public CharToByteCp861() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
