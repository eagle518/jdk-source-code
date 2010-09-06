/*
 * @(#)ByteToCharCp874.java	1.18	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM874;

/**
 * A table to convert to Cp874 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp874 extends ByteToCharSingleByte {

    private final static IBM874 nioCoder = new IBM874();

    public String getCharacterEncoding() {
        return "Cp874";
    }

    public ByteToCharCp874() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
