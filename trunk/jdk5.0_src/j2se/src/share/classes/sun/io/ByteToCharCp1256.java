/*
 * @(#)ByteToCharCp1256.java	1.19	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MS1256;

/**
 * A table to convert Cp1256 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1256 extends ByteToCharSingleByte {

    private final static MS1256 nioCoder = new MS1256();

    public String getCharacterEncoding() {
        return "Cp1256";
    }

    public ByteToCharCp1256() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
