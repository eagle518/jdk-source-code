/*
 * @(#)ByteToCharCp1254.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1254;

/**
 * A table to convert Cp1254 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1254 extends ByteToCharSingleByte {

    private final static MS1254 nioCoder = new MS1254();

    public String getCharacterEncoding() {
        return "Cp1254";
    }

    public ByteToCharCp1254() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
