/*
 * @(#)ByteToCharCp857.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM857;

/**
 * A table to convert to Cp857 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp857 extends ByteToCharSingleByte {

    private final static IBM857 nioCoder = new IBM857();

    public String getCharacterEncoding() {
        return "Cp857";
    }

    public ByteToCharCp857() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
