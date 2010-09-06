/*
 * @(#)ByteToCharCp850.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM850;

/**
 * A table to convert to Cp850 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp850 extends ByteToCharSingleByte {

    public String getCharacterEncoding() {
        return "Cp850";
    }

    public ByteToCharCp850() {
        super.byteToCharTable = IBM850.getDecoderSingleByteMappings();
    }
}
