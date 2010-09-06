/*
 * @(#)CharToByteMS936.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MS936;

/**
 * Tables and data to convert Unicode to MS936
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteMS936 extends CharToByteDoubleByte {

    private final static MS936 nioCoder = new MS936();

    public String getCharacterEncoding() {
        return "MS936";
    }

    public CharToByteMS936() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
