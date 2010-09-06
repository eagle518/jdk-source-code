/*
 * @(#)ByteToCharCp1025.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1025;

/**
 * A table to convert to Cp1025 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1025 extends ByteToCharSingleByte {

    private final static IBM1025 nioCoder = new IBM1025();

    public String getCharacterEncoding() {
        return "Cp1025";
    }

    public ByteToCharCp1025() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
