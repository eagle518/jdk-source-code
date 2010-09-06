/*
 * @(#)ByteToCharMacGreek.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacGreek;

/**
 * A table to convert to MacGreek to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacGreek extends ByteToCharSingleByte {

    private final static MacGreek nioCoder = new MacGreek();

    public String getCharacterEncoding() {
        return "MacGreek";
    }

    public ByteToCharMacGreek() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
