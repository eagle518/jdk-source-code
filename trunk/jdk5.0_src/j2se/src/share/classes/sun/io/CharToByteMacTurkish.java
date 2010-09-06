/*
 * @(#)CharToByteMacTurkish.java	1.13	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MacTurkish;

/**
 * Tables and data to convert Unicode to MacTurkish
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteMacTurkish extends CharToByteSingleByte {

    private final static MacTurkish nioCoder = new MacTurkish();

    public String getCharacterEncoding() {
        return "MacTurkish";
    }

    public CharToByteMacTurkish() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
