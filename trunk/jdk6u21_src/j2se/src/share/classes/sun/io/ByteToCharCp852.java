/*
 * @(#)ByteToCharCp852.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.IBM852;

/**
 * A table to convert to Cp852 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp852 extends ByteToCharSingleByte {

    private final static IBM852 nioCoder = new IBM852();

    public String getCharacterEncoding() {
        return "Cp852";
    }

    public ByteToCharCp852() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
