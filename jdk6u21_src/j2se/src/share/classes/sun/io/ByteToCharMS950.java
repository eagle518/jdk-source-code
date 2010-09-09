/*
 * @(#)ByteToCharMS950.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS950;

/**
 * Tables and data to convert MS950 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMS950 extends ByteToCharDoubleByte {

    private final static MS950 nioCoder = new MS950();

    public String getCharacterEncoding() {
        return "MS950";
    }

    public ByteToCharMS950() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0x40;
        end = 0xFE;
    }
}
