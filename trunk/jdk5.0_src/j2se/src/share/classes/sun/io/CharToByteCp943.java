/*
 * @(#)CharToByteCp943.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
// Table from Unicode to Cp943
package sun.io;

import sun.nio.cs.ext.IBM943;

/**
 * Tables and data to convert Unicode to Cp943
 *
 * @author  BuildTables tool
 */

public class CharToByteCp943 extends CharToByteDBCS_ASCII {

    private static IBM943 nioCoder = new IBM943();

    public String getCharacterEncoding() {
        return "Cp943";
    }

    public CharToByteCp943() {
        super();
        super.mask1 = 0xFFC0;
        super.mask2 = 0x003F;
        super.shift = 6;
        super.index1 = nioCoder.getEncoderIndex1();
        super.index2 = nioCoder.getEncoderIndex2();
        super.index2a = nioCoder.getEncoderIndex2a();
        subBytes = new byte[1];
        subBytes[0] = 0x6f;
    }
}

