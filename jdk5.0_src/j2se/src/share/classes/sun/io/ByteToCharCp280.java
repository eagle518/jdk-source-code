/*
 * @(#)ByteToCharCp280.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM280;

/**
 * A table to convert to Cp280 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp280 extends ByteToCharSingleByte {

    private final static IBM280 nioCoder = new IBM280();

    public String getCharacterEncoding() {
        return "Cp280";
    }

    public ByteToCharCp280() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
