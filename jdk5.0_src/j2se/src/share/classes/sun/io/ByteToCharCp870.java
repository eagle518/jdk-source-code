/*
 * @(#)ByteToCharCp870.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM870;

/**
 * A table to convert to Cp870 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp870 extends ByteToCharSingleByte {

    private final static IBM870 nioCoder = new IBM870();

    public String getCharacterEncoding() {
        return "Cp870";
    }

    public ByteToCharCp870() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
