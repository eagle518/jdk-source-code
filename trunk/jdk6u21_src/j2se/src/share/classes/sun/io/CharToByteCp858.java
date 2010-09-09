/*
 * @(#)CharToByteCp858.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 1998 - All Rights Reserved
 *
 */

package sun.io;

import sun.nio.cs.IBM858;

/**
 * Tables and data to convert Unicode to Cp858
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.7
 */

public class CharToByteCp858 extends CharToByteSingleByte {

    public String getCharacterEncoding() {
        return "Cp858";
    }

    private final static IBM858 nioCoder = new IBM858();

    public CharToByteCp858() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
