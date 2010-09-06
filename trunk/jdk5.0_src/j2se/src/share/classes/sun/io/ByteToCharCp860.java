/*
 * @(#)ByteToCharCp860.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM860;

/**
 * A table to convert to Cp860 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp860 extends ByteToCharSingleByte {

    private final static IBM860 nioCoder = new IBM860();

    public String getCharacterEncoding() {
        return "Cp860";
    }

    public ByteToCharCp860() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
