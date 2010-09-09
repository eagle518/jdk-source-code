/*
 * @(#)CharToByteMacIceland.java	1.15	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MacIceland;

/**
 * Tables and data to convert Unicode to MacIceland
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteMacIceland extends CharToByteSingleByte {

    private final static MacIceland nioCoder = new MacIceland();

    public String getCharacterEncoding() {
        return "MacIceland";
    }

    public CharToByteMacIceland() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
