/*
 * @(#)CharToByteMS950.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS950;

/**
 * Tables and data to convert Unicode to MS950
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteMS950 extends CharToByteDoubleByte {

    private final static MS950 nioCoder = new MS950();

    public String getCharacterEncoding() {
        return "MS950";
    }

    public CharToByteMS950() {
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
