/*
 * @(#)ByteToCharISO8859_9.java	1.16 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ISO_8859_9;

/**
 * A table to convert ISO8859_9 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharISO8859_9 extends ByteToCharSingleByte {

    private final static ISO_8859_9 nioCoder = new ISO_8859_9();

    public String getCharacterEncoding() {
        return "ISO8859_9";
    }

    public ByteToCharISO8859_9() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
