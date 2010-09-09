/*
 * @(#)ByteToCharCp874.java	1.21	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.IBM874;

/**
 * A table to convert to Cp874 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp874 extends ByteToCharSingleByte {

    private final static IBM874 nioCoder = new IBM874();

    public String getCharacterEncoding() {
        return "Cp874";
    }

    public ByteToCharCp874() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
