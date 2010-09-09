/*
 * @(#)ByteToCharCp297.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM297;

/**
 * A table to convert to Cp297 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp297 extends ByteToCharSingleByte {

    private final static IBM297 nioCoder = new IBM297();

    public String getCharacterEncoding() {
        return "Cp297";
    }

    public ByteToCharCp297() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
