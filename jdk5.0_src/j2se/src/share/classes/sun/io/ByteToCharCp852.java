/*
 * @(#)ByteToCharCp852.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM852;

/**
 * A table to convert to Cp852 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp852 extends ByteToCharSingleByte {

    private final static IBM852 nioCoder = new IBM852();

    public String getCharacterEncoding() {
        return "Cp852";
    }

    public ByteToCharCp852() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
