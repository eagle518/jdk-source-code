/*
 * @(#)ByteToCharCp918.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM918;

/**
 * A table to convert to Cp918 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp918 extends ByteToCharSingleByte {

    private final static IBM918 nioCoder = new IBM918();

    public String getCharacterEncoding() {
        return "Cp918";
    }

    public ByteToCharCp918() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
