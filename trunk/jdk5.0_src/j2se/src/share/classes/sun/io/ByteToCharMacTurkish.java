/*
 * @(#)ByteToCharMacTurkish.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacTurkish;

/**
 * A table to convert to MacTurkish to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacTurkish extends ByteToCharSingleByte {

    private final static MacTurkish nioCoder = new MacTurkish();

    public String getCharacterEncoding() {
        return "MacTurkish";
    }

    public ByteToCharMacTurkish() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
