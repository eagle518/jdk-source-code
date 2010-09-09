/*
 * @(#)ByteToCharMacIceland.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacIceland;

/**
 * A table to convert to MacIceland to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacIceland extends ByteToCharSingleByte {

    private final static MacIceland nioCoder = new MacIceland();

    public String getCharacterEncoding() {
        return "MacIceland";
    }

    public ByteToCharMacIceland() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
