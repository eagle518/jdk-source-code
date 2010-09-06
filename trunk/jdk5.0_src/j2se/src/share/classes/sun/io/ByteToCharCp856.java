/*
 * @(#)ByteToCharCp856.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM856;

/**
 * A table to convert to Cp856 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp856 extends ByteToCharSingleByte {

    private final static IBM856 nioCoder = new IBM856();

    public String getCharacterEncoding() {
        return "Cp856";
    }

    public ByteToCharCp856() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
