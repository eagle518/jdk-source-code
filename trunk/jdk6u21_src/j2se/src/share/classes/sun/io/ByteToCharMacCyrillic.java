/*
 * @(#)ByteToCharMacCyrillic.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacCyrillic;

/**
 * A table to convert to MacCyrillic to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacCyrillic extends ByteToCharSingleByte {

    private final static MacCyrillic nioCoder = new MacCyrillic();

    public String getCharacterEncoding() {
        return "MacCyrillic";
    }

    public ByteToCharMacCyrillic() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
