/*
 * @(#)ByteToCharCp921.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM921;

/**
 * A table to convert to Cp921 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp921 extends ByteToCharSingleByte {

    private final static IBM921 nioCoder = new IBM921();

    public String getCharacterEncoding() {
        return "Cp921";
    }

    public ByteToCharCp921() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
