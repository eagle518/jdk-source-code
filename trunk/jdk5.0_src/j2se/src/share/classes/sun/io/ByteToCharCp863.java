/*
 * @(#)ByteToCharCp863.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM863;

/**
 * A table to convert to Cp863 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp863 extends ByteToCharSingleByte {

    private final static IBM863 nioCoder = new IBM863();

    public String getCharacterEncoding() {
        return "Cp863";
    }

    public ByteToCharCp863() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
