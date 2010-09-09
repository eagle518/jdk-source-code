/*
 * @(#)CharToByteEUC_CN.java	1.22 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.EUC_CN;

/**
 * Tables and data to convert Unicode to EUC_CN
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteEUC_CN extends CharToByteDoubleByte {

    private final static EUC_CN nioCoder = new EUC_CN();

    public String getCharacterEncoding() {
        return "EUC_CN";
    }

    public CharToByteEUC_CN() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
