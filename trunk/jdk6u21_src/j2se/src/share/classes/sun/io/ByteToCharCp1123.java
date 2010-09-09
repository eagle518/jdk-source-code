/*
 * @(#)ByteToCharCp1123.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1123;

/**
 * A table to convert to Cp1123 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1123 extends ByteToCharSingleByte {

    private final static IBM1123 nioCoder = new IBM1123();

    public String getCharacterEncoding() {
        return "Cp1123";
    }

    public ByteToCharCp1123() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
