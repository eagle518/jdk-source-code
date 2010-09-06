/*
 * @(#)ByteToCharMacRoman.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacRoman;

/**
 * A table to convert to MacRoman to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacRoman extends ByteToCharSingleByte {

    private final static MacRoman nioCoder = new MacRoman();

    public String getCharacterEncoding() {
        return "MacRoman";
    }

    public ByteToCharMacRoman() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
