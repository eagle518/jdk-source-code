/*
 * @(#)ByteToCharCp1122.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1122;

/**
 * A table to convert to Cp1122 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1122 extends ByteToCharSingleByte {

    private final static IBM1122 nioCoder = new IBM1122();

    public String getCharacterEncoding() {
        return "Cp1122";
    }

    public ByteToCharCp1122() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
