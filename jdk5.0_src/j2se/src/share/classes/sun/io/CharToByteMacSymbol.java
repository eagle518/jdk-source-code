/*
 * @(#)CharToByteMacSymbol.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MacSymbol;

/**
 * Tables and data to convert Unicode to MacSymbol
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class CharToByteMacSymbol extends CharToByteSingleByte {

    private final static MacSymbol nioCoder = new MacSymbol();

    public String getCharacterEncoding() {
        return "MacSymbol";
    }

    public CharToByteMacSymbol() {
        super.mask1 = 0xFF00;
        super.mask2 = 0x00FF;
        super.shift = 8;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
    }
}
