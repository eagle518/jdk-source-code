/*
 * @(#)ByteToCharCp1254.java	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1254;

/**
 * A table to convert Cp1254 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1254 extends ByteToCharSingleByte {

    private final static MS1254 nioCoder = new MS1254();

    public String getCharacterEncoding() {
        return "Cp1254";
    }

    public ByteToCharCp1254() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
