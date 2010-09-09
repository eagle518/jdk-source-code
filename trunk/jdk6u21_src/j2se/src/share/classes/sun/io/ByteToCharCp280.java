/*
 * @(#)ByteToCharCp280.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM280;

/**
 * A table to convert to Cp280 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp280 extends ByteToCharSingleByte {

    private final static IBM280 nioCoder = new IBM280();

    public String getCharacterEncoding() {
        return "Cp280";
    }

    public ByteToCharCp280() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
