/*
 * @(#)ByteToCharCp273.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM273;

/**
 * A table to convert to Cp273 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp273 extends ByteToCharSingleByte {

    private final static IBM273 nioCoder = new IBM273();

    public String getCharacterEncoding() {
        return "Cp273";
    }

    public ByteToCharCp273() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
