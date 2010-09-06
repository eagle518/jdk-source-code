/*
 * @(#)ByteToCharCp1123.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1123;

/**
 * A table to convert to Cp1123 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1123 extends ByteToCharSingleByte {

    private final static IBM1123 nioCoder = new IBM1123();

    public String getCharacterEncoding() {
        return "Cp1123";
    }

    public ByteToCharCp1123() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
