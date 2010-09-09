/*
 * @(#)ByteToCharCp860.java	1.16	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM860;

/**
 * A table to convert to Cp860 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp860 extends ByteToCharSingleByte {

    private final static IBM860 nioCoder = new IBM860();

    public String getCharacterEncoding() {
        return "Cp860";
    }

    public ByteToCharCp860() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
