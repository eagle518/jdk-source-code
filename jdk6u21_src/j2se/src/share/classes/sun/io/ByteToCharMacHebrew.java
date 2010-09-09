/*
 * @(#)ByteToCharMacHebrew.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacHebrew;

/**
 * A table to convert to MacHebrew to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacHebrew extends ByteToCharSingleByte {

    private final static MacHebrew nioCoder = new MacHebrew();

    public String getCharacterEncoding() {
        return "MacHebrew";
    }

    public ByteToCharMacHebrew() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
