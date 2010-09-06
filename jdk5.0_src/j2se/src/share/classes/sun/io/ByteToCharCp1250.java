/*
 * @(#)ByteToCharCp1250.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.MS1250;

/**
 * A table to convert Cp1250 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1250 extends ByteToCharSingleByte {

    private final static MS1250 nioCoder = new MS1250();

    public String getCharacterEncoding() {
        return "Cp1250";
    }

    public ByteToCharCp1250() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
