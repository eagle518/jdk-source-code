/*
 * @(#)ByteToCharCp437.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.IBM437;

/**
 * A table to convert to Cp437 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp437 extends ByteToCharSingleByte {

    private final static IBM437 nioCoder = new IBM437();

    public String getCharacterEncoding() {
        return "Cp437";
    }

    public ByteToCharCp437() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
