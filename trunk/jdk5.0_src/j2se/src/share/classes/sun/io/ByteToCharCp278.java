/*
 * @(#)ByteToCharCp278.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM278;

/**
 * A table to convert to Cp278 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp278 extends ByteToCharSingleByte {

    private final static IBM278 nioCoder = new IBM278();

    public String getCharacterEncoding() {
        return "Cp278";
    }

    public ByteToCharCp278() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
