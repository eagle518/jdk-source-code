/*
 * @(#)ByteToCharCp1047.java	1.4	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM1047;

/**
 * A table to convert to Cp1047 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp1047 extends ByteToCharSingleByte {

    private final static IBM1047 nioCoder = new IBM1047();

    public String getCharacterEncoding() {
        return "Cp1047";
    }

    public ByteToCharCp1047() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
