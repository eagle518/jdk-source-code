/*
 * @(#)ByteToCharCp1258.java	1.20	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS1258;

/**
 * A table to convert Cp1258 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1258 extends ByteToCharSingleByte {

    private final static MS1258 nioCoder = new MS1258();

    public String getCharacterEncoding() {
        return "Cp1258";
    }

    public ByteToCharCp1258() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
