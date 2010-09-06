/*
 * @(#)ByteToCharCp277.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM277;

/**
 * A table to convert to Cp277 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp277 extends ByteToCharSingleByte {

    private final static IBM277 nioCoder = new IBM277();

    public String getCharacterEncoding() {
        return "Cp277";
    }

    public ByteToCharCp277() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
