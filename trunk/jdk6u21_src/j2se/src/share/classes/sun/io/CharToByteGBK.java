/*
 * @(#)CharToByteGBK.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.GBK;

/**
 * Tables and data to convert Unicode to GBK
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteGBK extends CharToByteDoubleByte {

    private final static GBK nioCoder = new GBK();

    public String getCharacterEncoding() {
        return "GBK";
    }

    public CharToByteGBK() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
