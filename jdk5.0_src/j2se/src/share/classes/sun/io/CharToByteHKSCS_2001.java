/*
 * @(#)CharToByteHKSCS_2001.java	1.2	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



package sun.io;

import sun.nio.cs.ext.HKSCS_2001;

/**
 * Tables and data to convert Unicode to HKSCS (2001 revision)
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteHKSCS_2001 extends CharToByteDoubleByte {

    public String getCharacterEncoding() {
        return "HKSCS_2001";
    }

    public CharToByteHKSCS_2001() {
        super.index1 = HKSCS_2001.getEncoderIndex1();
        super.index2 = HKSCS_2001.getEncoderIndex2();
    }
}
