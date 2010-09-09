/*
 * @(#)ByteToCharCp285.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM285;

/**
 * A table to convert to Cp285 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp285 extends ByteToCharSingleByte {

    private final static IBM285 nioCoder = new IBM285();

    public String getCharacterEncoding() {
        return "Cp285";
    }

    public ByteToCharCp285() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
