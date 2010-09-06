/*
 * @(#)CharToByteBig5.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.Big5;

/**
 * Tables and data to convert Unicode to Big5
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteBig5 extends CharToByteDoubleByte {

    private static final Big5 nioCoder = new Big5();

    public String getCharacterEncoding() {
        return "Big5";
    }

    public CharToByteBig5() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
