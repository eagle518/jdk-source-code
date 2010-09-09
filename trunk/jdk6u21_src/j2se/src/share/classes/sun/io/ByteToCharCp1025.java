/*
 * @(#)ByteToCharCp1025.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1025;

/**
 * A table to convert to Cp1025 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1025 extends ByteToCharSingleByte {

    private final static IBM1025 nioCoder = new IBM1025();

    public String getCharacterEncoding() {
        return "Cp1025";
    }

    public ByteToCharCp1025() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
