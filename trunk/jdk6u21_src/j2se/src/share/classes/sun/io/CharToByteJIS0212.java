/*
 * @(#)CharToByteJIS0212.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;
import sun.nio.cs.ext.JIS_X_0212_Encoder;

/**
 * Tables and data to convert Unicode to JIS0212
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteJIS0212 extends CharToByteDoubleByte {
    public String getCharacterEncoding() {
        return "JIS0212";
    }

    public CharToByteJIS0212() {
        super.index1 = JIS_X_0212_Encoder.getIndex1();
        super.index2 = JIS_X_0212_Encoder.getIndex2();
    }
}
