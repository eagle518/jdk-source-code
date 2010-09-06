/*
 * @(#)CharToByteHKSCS.java	1.8	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



package sun.io;

import sun.nio.cs.ext.HKSCS;

/**
 * Tables and data to convert Unicode to HKSCS
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteHKSCS extends CharToByteDoubleByte {

    public String getCharacterEncoding() {
        return "HKSCS";
    }

    public CharToByteHKSCS() {
        super.index1 = HKSCS.getEncoderIndex1();
        super.index2 = HKSCS.getEncoderIndex2();
    }
}
