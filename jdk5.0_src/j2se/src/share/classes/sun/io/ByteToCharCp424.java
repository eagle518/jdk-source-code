/*
 * @(#)ByteToCharCp424.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM424;

/**
 * A table to convert to Cp424 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp424 extends ByteToCharSingleByte {

    private final static IBM424 nioCoder = new IBM424();

    public String getCharacterEncoding() {
        return "Cp424";
    }

    public ByteToCharCp424() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
