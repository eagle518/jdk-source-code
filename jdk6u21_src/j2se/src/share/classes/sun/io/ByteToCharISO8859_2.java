/*
 * @(#)ByteToCharISO8859_2.java	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ISO_8859_2;

/**
 * A table to convert ISO8859_2 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharISO8859_2 extends ByteToCharSingleByte {

    private final static ISO_8859_2 nioCoder = new ISO_8859_2();

    public String getCharacterEncoding() {
        return "ISO8859_2";
    }

    public ByteToCharISO8859_2() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
