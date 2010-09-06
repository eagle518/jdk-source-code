/*
 * @(#)ByteToCharCp864.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM864;

/**
 * A table to convert to Cp864 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp864 extends ByteToCharSingleByte {

    private final static IBM864 nioCoder = new IBM864();

    public String getCharacterEncoding() {
        return "Cp864";
    }

    public ByteToCharCp864() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
