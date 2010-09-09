/*
 * @(#)ByteToCharCp870.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM870;

/**
 * A table to convert to Cp870 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp870 extends ByteToCharSingleByte {

    private final static IBM870 nioCoder = new IBM870();

    public String getCharacterEncoding() {
        return "Cp870";
    }

    public ByteToCharCp870() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
