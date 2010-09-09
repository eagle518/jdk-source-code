/*
 * @(#)ByteToCharCp1255.java	1.21	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS1255;

/**
 * A table to convert Cp1255 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1255 extends ByteToCharSingleByte {

    private final static MS1255 nioCoder = new MS1255();

    public String getCharacterEncoding() {
        return "Cp1255";
    }

    public ByteToCharCp1255() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
