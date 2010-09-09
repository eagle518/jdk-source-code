/*
 * @(#)ByteToCharMacDingbat.java	1.16	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacDingbat;

/**
 * A table to convert to MacDingbat to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacDingbat extends ByteToCharSingleByte {

    private final static MacDingbat nioCoder = new MacDingbat();

    public String getCharacterEncoding() {
        return "MacDingbat";
    }

    public ByteToCharMacDingbat() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
