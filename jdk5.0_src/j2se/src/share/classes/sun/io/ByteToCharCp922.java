/*
 * @(#)ByteToCharCp922.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM922;

/**
 * A table to convert to Cp922 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp922 extends ByteToCharSingleByte {

    private final static IBM922 nioCoder = new IBM922();

    public String getCharacterEncoding() {
        return "Cp922";
    }

    public ByteToCharCp922() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
