/*
 * @(#)CharToByteEUC_KR.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.EUC_KR;

/**
 * Tables and data to convert Unicode to EUC_KR
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteEUC_KR extends CharToByteDoubleByte {

    private final static EUC_KR nioCoder = new EUC_KR();

    public String getCharacterEncoding() {
        return "EUC_KR";
    }

    public CharToByteEUC_KR() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
