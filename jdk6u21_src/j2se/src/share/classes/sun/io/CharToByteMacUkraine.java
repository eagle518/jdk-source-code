/*
 * @(#)CharToByteMacUkraine.java	1.15	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MacUkraine;

/**
 * Tables and data to convert Unicode to MacUkraine
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteMacUkraine extends CharToByteSingleByte {

    private final static MacUkraine nioCoder = new MacUkraine();

    public String getCharacterEncoding() {
        return "MacUkraine";
    }

    public CharToByteMacUkraine() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
