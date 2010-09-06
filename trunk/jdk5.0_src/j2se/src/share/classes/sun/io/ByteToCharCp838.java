/*
 * @(#)ByteToCharCp838.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM838;

/**
 * A table to convert to Cp838 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp838 extends ByteToCharSingleByte {

    private final static IBM838 nioCoder = new IBM838();

    public String getCharacterEncoding() {
        return "Cp838";
    }

    public ByteToCharCp838() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
