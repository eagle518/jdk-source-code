/*
 * @(#)CharToByteCp1144.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 1998 - All Rights Reserved
 *
 */

package sun.io;

import sun.nio.cs.ext.IBM1144;

/**
 * Tables and data to convert Unicode to Cp1144
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.7
 */

public class CharToByteCp1144 extends CharToByteSingleByte {

    private final static IBM1144 nioCoder = new IBM1144();

    public String getCharacterEncoding() {
        return "Cp1144";
    }

    public CharToByteCp1144() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
