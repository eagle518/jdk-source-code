/*
 * @(#)ByteToCharCp1124.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1124;

/**
 * A table to convert to Cp1124 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1124 extends ByteToCharSingleByte {

    private final static IBM1124 nioCoder = new IBM1124();

    public String getCharacterEncoding() {
        return "Cp1124";
    }

    public ByteToCharCp1124() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
