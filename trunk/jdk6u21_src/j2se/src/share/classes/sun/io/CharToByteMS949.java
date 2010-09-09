/*
 * @(#)CharToByteMS949.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS949;

/**
 * Tables and data to convert Unicode to MS949
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteMS949 extends CharToByteDoubleByte {

    private final static MS949 nioCoder = new MS949();

    public String getCharacterEncoding() {
        return "MS949";
    }

    public CharToByteMS949() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
