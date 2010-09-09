/*
 * @(#)ByteToCharMS874.java	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS874;

/**
 * A table to convert MS874 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMS874 extends ByteToCharSingleByte {

    private final static MS874 nioCoder = new MS874();

    public String getCharacterEncoding() {
        return "MS874";
    }

    public ByteToCharMS874() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
