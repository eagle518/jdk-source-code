/*
 * @(#)ByteToCharCp297.java	1.16	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM297;

/**
 * A table to convert to Cp297 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp297 extends ByteToCharSingleByte {

    private final static IBM297 nioCoder = new IBM297();

    public String getCharacterEncoding() {
        return "Cp297";
    }

    public ByteToCharCp297() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
