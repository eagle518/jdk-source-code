/*
 * @(#)ByteToCharCp1112.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1112;

/**
 * A table to convert to Cp1112 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1112 extends ByteToCharSingleByte {

    private final static IBM1112 nioCoder = new IBM1112();

    public String getCharacterEncoding() {
        return "Cp1112";
    }

    public ByteToCharCp1112() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
