/*
 * @(#)ByteToCharCp420.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM420;

/**
 * A table to convert to Cp420 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp420 extends ByteToCharSingleByte {

    private final static IBM420 nioCoder = new IBM420();

    public String getCharacterEncoding() {
        return "Cp420";
    }

    public ByteToCharCp420() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
