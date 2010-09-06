/*
 * @(#)ByteToCharMacRomania.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacRomania;

/**
 * A table to convert to MacRomania to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacRomania extends ByteToCharSingleByte {

    private final static MacRomania nioCoder = new MacRomania();

    public String getCharacterEncoding() {
        return "MacRomania";
    }

    public ByteToCharMacRomania() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
