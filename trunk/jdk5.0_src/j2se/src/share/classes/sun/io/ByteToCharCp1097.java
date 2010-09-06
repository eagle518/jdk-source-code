/*
 * @(#)ByteToCharCp1097.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1097;

/**
 * A table to convert to Cp1097 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1097 extends ByteToCharSingleByte {

    private final static IBM1097 nioCoder = new IBM1097();

    public String getCharacterEncoding() {
        return "Cp1097";
    }

    public ByteToCharCp1097() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
