/*
 * @(#)ByteToCharGBK.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.GBK;

/**
 * Tables and data to convert GBK to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharGBK extends ByteToCharDoubleByte {

    private final static GBK nioCoder = new GBK();

    public String getCharacterEncoding() {
        return "GBK";
    }

    public ByteToCharGBK() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0x40;
        end = 0xFE;
    }
}
