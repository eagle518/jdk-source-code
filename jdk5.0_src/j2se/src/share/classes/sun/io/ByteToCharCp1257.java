/*
 * @(#)ByteToCharCp1257.java	1.17	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1257;

/**
 * A table to convert Cp1257 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1257 extends ByteToCharSingleByte {

    private final static MS1257 nioCoder = new MS1257();

    public String getCharacterEncoding() {
        return "Cp1257";
    }

    public ByteToCharCp1257() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
