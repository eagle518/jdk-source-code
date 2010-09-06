/*
 * @(#)CharToByteGBK.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
