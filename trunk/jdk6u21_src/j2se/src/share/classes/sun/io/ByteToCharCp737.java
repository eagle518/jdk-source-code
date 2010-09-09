/*
 * @(#)ByteToCharCp737.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.IBM737;

/**
 * A table to convert to Cp737 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp737 extends ByteToCharSingleByte {

    private final static IBM737 nioCoder = new IBM737();

    public String getCharacterEncoding() {
        return "Cp737";
    }

    public ByteToCharCp737() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
