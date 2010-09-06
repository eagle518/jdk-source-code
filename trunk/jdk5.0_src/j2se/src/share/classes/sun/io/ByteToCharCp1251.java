/*
 * @(#)ByteToCharCp1251.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1251;

/**
 * A table to convert Cp1251 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1251 extends ByteToCharSingleByte {

    private final static MS1251 nioCoder = new MS1251();

    public String getCharacterEncoding() {
        return "Cp1251";
    }

    public ByteToCharCp1251() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
