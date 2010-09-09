/*
 * @(#)CharToByteHKSCS.java	1.10	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
