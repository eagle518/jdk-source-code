/*
 * @(#)ByteToCharCp037.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM037;

/**
 * A table to convert to Cp037 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp037 extends ByteToCharSingleByte {

    private final static IBM037 nioCoder = new IBM037();

    public String getCharacterEncoding() {
        return "Cp037";
    }

    public ByteToCharCp037() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
