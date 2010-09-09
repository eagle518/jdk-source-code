/*
 * @(#)ByteToCharMacCroatian.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacCroatian;

/**
 * A table to convert to MacCroatian to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacCroatian extends ByteToCharSingleByte {

    private final static MacCroatian nioCoder = new MacCroatian();

    public String getCharacterEncoding() {
        return "MacCroatian";
    }

    public ByteToCharMacCroatian() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
