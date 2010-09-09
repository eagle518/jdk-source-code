/*
 * @(#)ByteToCharEUC_CN.java	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.EUC_CN;

/**
 * Tables and data to convert EUC_CN to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharEUC_CN extends ByteToCharDoubleByte {

    private EUC_CN nioCoder = new EUC_CN();

    public String getCharacterEncoding() {
        return "EUC_CN";
    }

    public ByteToCharEUC_CN() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0xA1;
        end = 0xFE;
    }
}
