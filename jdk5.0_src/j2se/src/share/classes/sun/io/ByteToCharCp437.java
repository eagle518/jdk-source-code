/*
 * @(#)ByteToCharCp437.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM437;

/**
 * A table to convert to Cp437 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp437 extends ByteToCharSingleByte {

    private final static IBM437 nioCoder = new IBM437();

    public String getCharacterEncoding() {
        return "Cp437";
    }

    public ByteToCharCp437() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
