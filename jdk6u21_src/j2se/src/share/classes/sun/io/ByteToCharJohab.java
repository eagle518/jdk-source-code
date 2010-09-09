/*
 * @(#)ByteToCharJohab.java	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.Johab;

/**
 * Tables and data to convert Johab to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharJohab extends ByteToCharDoubleByte {

    private final static Johab nioCoder = new Johab();

    public String getCharacterEncoding() {
        return "Johab";
    }

    public ByteToCharJohab() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0x20;
        end = 0xFE;
    }
}
