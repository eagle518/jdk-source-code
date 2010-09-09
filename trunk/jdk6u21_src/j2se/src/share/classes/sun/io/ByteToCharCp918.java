/*
 * @(#)ByteToCharCp918.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM918;

/**
 * A table to convert to Cp918 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp918 extends ByteToCharSingleByte {

    private final static IBM918 nioCoder = new IBM918();

    public String getCharacterEncoding() {
        return "Cp918";
    }

    public ByteToCharCp918() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
