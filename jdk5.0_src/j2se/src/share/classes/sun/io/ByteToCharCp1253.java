/*
 * @(#)ByteToCharCp1253.java	1.17	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1253;

/**
 * A table to convert Cp1253 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1253 extends ByteToCharSingleByte {

    private final static MS1253 nioCoder = new MS1253();

    public String getCharacterEncoding() {
        return "Cp1253";
    }

    public ByteToCharCp1253() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
