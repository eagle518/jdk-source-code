/*
 * @(#)ByteToCharCp921.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM921;

/**
 * A table to convert to Cp921 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp921 extends ByteToCharSingleByte {

    private final static IBM921 nioCoder = new IBM921();

    public String getCharacterEncoding() {
        return "Cp921";
    }

    public ByteToCharCp921() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
