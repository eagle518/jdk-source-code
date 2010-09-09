/*
 * @(#)ByteToCharEUC_KR.java	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.EUC_KR;

/**
 * Tables and data to convert EUC_KR to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharEUC_KR extends ByteToCharDoubleByte {

    private final static EUC_KR nioCoder = new EUC_KR();

    public String getCharacterEncoding() {
        return "EUC_KR";
    }

    public ByteToCharEUC_KR() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0xA1;
        end = 0xFE;
    }
}
