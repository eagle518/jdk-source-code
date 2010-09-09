/*
 * @(#)ByteToCharCp775.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.IBM775;

/**
 * A table to convert to Cp775 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp775 extends ByteToCharSingleByte {

    private final static IBM775 nioCoder = new IBM775();

    public String getCharacterEncoding() {
        return "Cp775";
    }

    public ByteToCharCp775() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
