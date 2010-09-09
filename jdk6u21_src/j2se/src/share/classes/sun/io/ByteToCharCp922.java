/*
 * @(#)ByteToCharCp922.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM922;

/**
 * A table to convert to Cp922 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp922 extends ByteToCharSingleByte {

    private final static IBM922 nioCoder = new IBM922();

    public String getCharacterEncoding() {
        return "Cp922";
    }

    public ByteToCharCp922() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
