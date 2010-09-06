/*
 * @(#)ByteToCharCp861.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM861;

/**
 * A table to convert to Cp861 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp861 extends ByteToCharSingleByte {

    private final static IBM861 nioCoder = new IBM861();

    public String getCharacterEncoding() {
        return "Cp861";
    }

    public ByteToCharCp861() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
