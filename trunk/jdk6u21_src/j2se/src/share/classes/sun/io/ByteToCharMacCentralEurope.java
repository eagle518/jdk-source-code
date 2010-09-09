/*
 * @(#)ByteToCharMacCentralEurope.java	1.17	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacCentralEurope;

/**
 * A table to convert to MacCentralEurope to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacCentralEurope extends ByteToCharSingleByte {

    private final static MacCentralEurope nioCoder = new MacCentralEurope();

    public String getCharacterEncoding() {
        return "MacCentralEurope";
    }

    public ByteToCharMacCentralEurope() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
