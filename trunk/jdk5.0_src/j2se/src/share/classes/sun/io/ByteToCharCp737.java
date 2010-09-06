/*
 * @(#)ByteToCharCp737.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM737;

/**
 * A table to convert to Cp737 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp737 extends ByteToCharSingleByte {

    private final static IBM737 nioCoder = new IBM737();

    public String getCharacterEncoding() {
        return "Cp737";
    }

    public ByteToCharCp737() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
