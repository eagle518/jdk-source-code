/*
 * @(#)ByteToCharCp871.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM871;

/**
 * A table to convert to Cp871 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp871 extends ByteToCharSingleByte {

    private final static IBM871 nioCoder = new IBM871();

    public String getCharacterEncoding() {
        return "Cp871";
    }

    public ByteToCharCp871() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
