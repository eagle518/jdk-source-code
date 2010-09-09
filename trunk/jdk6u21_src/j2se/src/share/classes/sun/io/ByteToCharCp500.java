/*
 * @(#)ByteToCharCp500.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM500;

/**
 * A table to convert to Cp500 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp500 extends ByteToCharSingleByte {

    private final static IBM500 nioCoder = new IBM500();

    public String getCharacterEncoding() {
        return "Cp500";
    }

    public ByteToCharCp500() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
