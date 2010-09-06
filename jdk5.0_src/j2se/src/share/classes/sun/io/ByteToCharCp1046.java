/*
 * @(#)ByteToCharCp1046.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1046;

/**
 * A table to convert to Cp1046 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1046 extends ByteToCharSingleByte {

    private final static IBM1046 nioCoder = new IBM1046();

    public String getCharacterEncoding() {
        return "Cp1046";
    }

    public ByteToCharCp1046() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
