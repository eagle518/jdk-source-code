/*
 * @(#)ByteToCharCp875.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM875;

/**
 * A table to convert to Cp875 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp875 extends ByteToCharSingleByte {

    private final static IBM875 nioCoder = new IBM875();

    public String getCharacterEncoding() {
        return "Cp875";
    }

    public ByteToCharCp875() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
