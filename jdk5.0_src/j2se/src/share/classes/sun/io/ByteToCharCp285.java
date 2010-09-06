/*
 * @(#)ByteToCharCp285.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM285;

/**
 * A table to convert to Cp285 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp285 extends ByteToCharSingleByte {

    private final static IBM285 nioCoder = new IBM285();

    public String getCharacterEncoding() {
        return "Cp285";
    }

    public ByteToCharCp285() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
