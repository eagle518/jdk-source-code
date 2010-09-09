/*
 * @(#)ByteToCharMacRoman.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacRoman;

/**
 * A table to convert to MacRoman to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacRoman extends ByteToCharSingleByte {

    private final static MacRoman nioCoder = new MacRoman();

    public String getCharacterEncoding() {
        return "MacRoman";
    }

    public ByteToCharMacRoman() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
