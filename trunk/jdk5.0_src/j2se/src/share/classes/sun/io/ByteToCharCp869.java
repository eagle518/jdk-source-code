/*
 * @(#)ByteToCharCp869.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM869;

/**
 * A table to convert to Cp869 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp869 extends ByteToCharSingleByte {

    private final static IBM869 nioCoder = new IBM869();

    public String getCharacterEncoding() {
        return "Cp869";
    }

    public ByteToCharCp869() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
