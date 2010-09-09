/*
 * @(#)ByteToCharMacThai.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacThai;

/**
 * A table to convert to MacThai to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacThai extends ByteToCharSingleByte {

    private final static MacThai nioCoder = new MacThai();

    public String getCharacterEncoding() {
        return "MacThai";
    }

    public ByteToCharMacThai() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
