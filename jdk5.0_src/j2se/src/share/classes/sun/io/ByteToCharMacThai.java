/*
 * @(#)ByteToCharMacThai.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacThai;

/**
 * A table to convert to MacThai to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacThai extends ByteToCharSingleByte {

    private final static MacThai nioCoder = new MacThai();

    public String getCharacterEncoding() {
        return "MacThai";
    }

    public ByteToCharMacThai() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
